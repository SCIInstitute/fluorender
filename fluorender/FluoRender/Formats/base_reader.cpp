#include <Formats\base_reader.h>

int BaseReader::LZWDecode(tidata_t tif, tidata_t op0, tsize_t occ0)
{
	//initialize codec state
	LZWCodecState *sp = new LZWCodecState;
	//sp->predictor = m_predictor;
	sp->stride = 1;
	//sp->rowsize = m_x_size;
	sp->dec_codetab = new code_t[CSIZE*sizeof(code_t)];
	int icode = 255;
	do
	{
		sp->dec_codetab[icode].value = icode;
		sp->dec_codetab[icode].firstchar = icode;
		sp->dec_codetab[icode].length = 1;
		sp->dec_codetab[icode].next = NULL;
	} while (icode--);
	sp->maxcode = MAXCODE(BITS_MIN)-1;
	sp->nbits = BITS_MIN;
	sp->nextbits = 0;
	sp->nextdata = 0;
	sp->dec_restart = 0;
	sp->dec_nbitsmask = MAXCODE(BITS_MIN);
	sp->dec_bitsleft = occ0 << 3;
	sp->dec_free_entp = sp->dec_codetab + CODE_FIRST;
	memset(sp->dec_free_entp, 0, (CSIZE-CODE_FIRST)*sizeof(code_t));
	sp->dec_oldcodep = &sp->dec_codetab[-1];
	sp->dec_maxcodep = &sp->dec_codetab[sp->dec_nbitsmask-1];

	char *op = (char*) op0;
	long occ = (long) occ0;
	char *tp;
	unsigned char *bp;
	hcode_t code;
	int len;
	long nbits, nextbits, nextdata, nbitsmask;
	code_t *codep, *free_entp, *maxcodep, *oldcodep;

	bp = (unsigned char *)tif;
	nbits = sp->nbits;
	nextdata = sp->nextdata;
	nextbits = sp->nextbits;
	nbitsmask = sp->dec_nbitsmask;
	oldcodep = sp->dec_oldcodep;
	free_entp = sp->dec_free_entp;
	maxcodep = sp->dec_maxcodep;

	while (occ > 0)
	{
		NextCode(tif, sp, bp, code, GetNextCode);
		if (code == CODE_EOI)
			break;
		if (code == CODE_CLEAR)
		{
			free_entp = sp->dec_codetab + CODE_FIRST;
			nbits = BITS_MIN;
			nbitsmask = MAXCODE(BITS_MIN);
			maxcodep = sp->dec_codetab + nbitsmask-1;
			NextCode(tif, sp, bp, code, GetNextCode);
			if (code == CODE_EOI)
				break;
			*op++ = (char)code, occ--;
			oldcodep = sp->dec_codetab + code;
			continue;
		}
		codep = sp->dec_codetab + code;

		/*
	 	 * Add the new entry to the code table.
	 	 */
		if (free_entp < &sp->dec_codetab[0] ||
			free_entp >= &sp->dec_codetab[CSIZE])
			return 0;

		free_entp->next = oldcodep;
		if (free_entp->next < &sp->dec_codetab[0] ||
			free_entp->next >= &sp->dec_codetab[CSIZE])
			return 0;

		free_entp->firstchar = free_entp->next->firstchar;
		free_entp->length = free_entp->next->length+1;
		free_entp->value = (codep < free_entp) ?
		    codep->firstchar : free_entp->firstchar;
		if (++free_entp > maxcodep)
		{
			if (++nbits > BITS_MAX)		/* should not happen */
				nbits = BITS_MAX;
			nbitsmask = MAXCODE(nbits);
			maxcodep = sp->dec_codetab + nbitsmask-1;
		}
		oldcodep = codep;
		if (code >= 256)
		{
			/*
			 * Code maps to a string, copy string
			 * value to output (written in reverse).
			 */
			if(codep->length == 0)
				return 0;
			if (codep->length > occ)
			{
				/*
				 * String is too long for decode buffer,
				 * locate portion that will fit, copy to
				 * the decode buffer, and setup restart
				 * logic for the next decoding call.
				 */
				sp->dec_codep = codep;
				do
				{
					codep = codep->next;
				} while (codep && codep->length > occ);
				if (codep)
				{
					sp->dec_restart = occ;
					tp = op + occ;
					do
					{
						*--tp = codep->value;
						codep = codep->next;
					} while (--occ && codep);
				}
				break;
			}
			len = codep->length;
			tp = op + len;
			do
			{
				int t;
				--tp;
				t = codep->value;
				codep = codep->next;
				*tp = t;
			} while (codep && tp > op);
			if (codep)
				break;
			op += len, occ -= len;
		} else
			*op++ = (char)code, occ--;
	}

	delete []sp->dec_codetab;
	delete sp;

	if (occ > 0)
		return 0;

	return (1);
}

void BaseReader::DecodeAcc8(tidata_t cp0, tsize_t cc)
{
	tsize_t stride = 1;

	char* cp = (char*) cp0;
	if (cc > stride)
	{
		cc -= stride;

		do
		{
			REPEAT4(stride, cp[stride] =
				(char) (cp[stride] + *cp); cp++)
			cc -= stride;
		} while ((int32) cc > 0);
	}
}

void BaseReader::DecodeAcc16(tidata_t cp0, tsize_t cc)
{
	tsize_t stride = 1;
	uint16* wp = (uint16*) cp0;
	tsize_t wc = cc / 2;

	if (wc > stride)
	{
		wc -= stride;
		do
		{
			REPEAT4(stride, wp[stride] += wp[0]; wp++)
			wc -= stride;
		} while ((int32) wc > 0);
	}
}

Nrrd* BaseReader::Convert(bool get_max) { return Convert(0,get_max); }

Nrrd* BaseReader::Convert(int c, bool get_max) { return Convert(0,c,get_max); }