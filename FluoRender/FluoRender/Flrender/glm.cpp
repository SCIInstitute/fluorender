/*
glm.c
Nate Robins, 1997, 2000
nate@pobox.com, http://www.pobox.com/~nate

Wavefront OBJ model file format reader/writer/manipulator.

Includes routines for generating smooth normals with
preservation of edges, welding redundant vertices & texture
coordinate generation (spheremap and planar projections) + more.

*/

#include <GL/glew.h>
#include <Color.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <glm.h>
#include <compatibility.h>

#define T(x) (model->triangles[(x)])
#define L(x) (model->lines[(x)])

char *sgets( char * str, int num, char **input );

/* _GLMnode: general purpose node */
typedef struct _GLMnode
{
	GLuint index;
	GLboolean averaged;
	struct _GLMnode* next;
} GLMnode;

GLboolean glmLoadTGA(TextureImage *texture, const char *filename,GLuint *textureID)      // Loads A TGA File Into Memory
{
	GLubyte    TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};  // Uncompressed TGA Header
	GLubyte    TGAcompare[12];                // Used To Compare TGA Header
	GLubyte    header[6];                  // First 6 Useful Bytes From The Header
	GLuint    bytesPerPixel;                // Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint    imageSize;                  // Used To Store The Image Size When Setting Aside Ram
	GLuint    temp;                    // Temporary Variable
	GLuint    type=GL_RGBA;                // Set The Default GL Mode To RBGA (32 BPP)

	FILE *file;
	std::string str = filename;
	if (!FOPEN(&file, str, "rb"))          // Open The TGA File
		return false;

	if(  file==NULL ||                    // Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||  // Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0        ||  // Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))        // If So Read Next 6 Header Bytes
	{
		if (file == NULL)                  // Did The File Even Exist? *Added Jim Strong*
			return false;                  // Return False
		else
		{
			fclose(file);                  // If Anything Failed, Close The File
			return false;                  // Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];      // Determine The TGA Width  (highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];      // Determine The TGA Height  (highbyte*256+lowbyte)

	if(  texture->width  <=0  ||                // Is The Width Less Than Or Equal To Zero
		texture->height  <=0  ||                // Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))          // Is The TGA 24 or 32 Bit?
	{
		fclose(file);                    // If Anything Failed, Close The File
		return false;                    // Return False
	}

	texture->bpp  = header[4];              // Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel  = texture->bpp/8;            // Divide By 8 To Get The Bytes Per Pixel
	imageSize    = texture->width*texture->height*bytesPerPixel;  // Calculate The Memory Required For The TGA Data

	texture->imageData=new GLubyte[imageSize];        // Reserve Memory To Hold The TGA Data

	if(  texture->imageData==NULL ||              // Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)  // Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)            // Was Image Data Loaded
			free(texture->imageData);            // If So, Release The Image Data

		fclose(file);                    // Close The File
		return false;                    // Return False
	}

	for(GLuint i=0; i<imageSize; i+=bytesPerPixel)    // Loop Through The Image Data
	{                            // Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];              // Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];  // Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;          // Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);                      // Close The File

	// Build A Texture From The Data
	glGenTextures(1, textureID);          // Generate OpenGL texture IDs

	glBindTexture(GL_TEXTURE_2D, *textureID);      // Bind Our Texture
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // Linear Filtered

	if (texture[0].bpp==24)                  // Was The TGA 24 Bits
	{
		type=GL_RGB;                    // If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	//release
	glBindTexture(GL_TEXTURE_2D, 0);
	return true;                      // Texture Building Went Ok, Return True
}

/* glmMax: returns the maximum of two floats */
static GLfloat glmMax(GLfloat a, GLfloat b)
{
	if (b > a)
		return b;
	return a;
}

/* glmAbs: returns the absolute value of a float */
static GLfloat glmAbs(GLfloat f)
{
	if (f < 0)
		return -f;
	return f;
}

/* glmDot: compute the dot product of two vectors
*
* u - array of 3 GLfloats (GLfloat u[3])
* v - array of 3 GLfloats (GLfloat v[3])
*/
static GLfloat glmDot(GLfloat* u, GLfloat* v)
{
	if (!u || !v)
		return 0;

	return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

/* glmCross: compute the cross product of two vectors
*
* u - array of 3 GLfloats (GLfloat u[3])
* v - array of 3 GLfloats (GLfloat v[3])
* n - array of 3 GLfloats (GLfloat n[3]) to return the cross product in
*/
static GLvoid glmCross(GLfloat* u, GLfloat* v, GLfloat* n)
{
	if (!u || !v || !n)
		return;

	n[0] = u[1]*v[2] - u[2]*v[1];
	n[1] = u[2]*v[0] - u[0]*v[2];
	n[2] = u[0]*v[1] - u[1]*v[0];
}

/* glmNormalize: normalize a vector
*
* v - array of 3 GLfloats (GLfloat v[3]) to be normalized
*/
static GLvoid glmNormalize(GLfloat* v)
{
	if (!v)
		return;

	GLfloat l;

	l = (GLfloat)sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	v[0] /= l;
	v[1] /= l;
	v[2] /= l;
}

/* glmEqual: compares two vectors and returns GL_TRUE if they are
* equal (within a certain threshold) or GL_FALSE if not. An epsilon
* that works fairly well is 0.000001.
*
* u - array of 3 GLfloats (GLfloat u[3])
* v - array of 3 GLfloats (GLfloat v[3])
*/
static GLboolean glmEqual(GLfloat* u, GLfloat* v, GLfloat epsilon)
{
	if (glmAbs(u[0] - v[0]) < epsilon &&
		glmAbs(u[1] - v[1]) < epsilon &&
		glmAbs(u[2] - v[2]) < epsilon)
	{
		return GL_TRUE;
	}
	return GL_FALSE;
}

/* glmWeldVectors: eliminate (weld) vectors that are within an
* epsilon of each other.
*
* vectors     - array of GLfloat[3]'s to be welded
* numvectors - number of GLfloat[3]'s in vectors
* epsilon     - maximum difference between vectors
*
*/
GLfloat* glmWeldVectors(GLfloat* vectors, GLuint* numvectors, GLfloat epsilon)
{
	GLfloat* copies;
	GLuint copied;
	GLuint i, j;

	copies = (GLfloat*)malloc(sizeof(GLfloat) * 3 * (*numvectors + 1));
	memcpy(copies, vectors, (sizeof(GLfloat) * 3 * (*numvectors + 1)));

	copied = 1;
	for (i = 1; i <= *numvectors; i++)
	{
		bool con = false;
		for (j = 1; j <= copied; j++)
		{
			if (glmEqual(&vectors[3 * i], &copies[3 * j], epsilon))
			{
				/* set the first component of this vector to point at the correct
				index into the new copies array */
				vectors[3 * i + 0] = (GLfloat)j;
				con = true;
				break;
			}
		}

		if (con)
			continue;

		/* must not be any duplicates -- add to the copies array */
		copied++;
		copies[3 * copied + 0] = vectors[3 * i + 0];
		copies[3 * copied + 1] = vectors[3 * i + 1];
		copies[3 * copied + 2] = vectors[3 * i + 2];
		j = copied;             /* pass this along for below */

		/* set the first component of this vector to point at the correct
		index into the new copies array */
		vectors[3 * i + 0] = (GLfloat)j;
	}

	*numvectors = copied;
	return copies;
}

/* glmFindGroup: Find a group in the model */
GLMgroup* glmFindGroup(GLMmodel* model, char* name)
{
	GLMgroup* group;

	assert(model);

	group = model->groups;
	while(group) {
		if (!strcmp(name, group->name))
			break;
		group = group->next;
	}

	return group;
}

/* glmAddGroup: Add a group to the model */
GLMgroup* glmAddGroup(GLMmodel* model, char* name)
{
	GLMgroup* group;

	group = glmFindGroup(model, name);
	if (!group)
	{
		group = (GLMgroup*)malloc(sizeof(GLMgroup));
		group->name = STRDUP(name);
		group->material = 0;
		group->numtriangles = 0;
		group->triangles = NULL;
		group->next = model->groups;
		model->groups = group;
		model->numgroups++;
	}

	return group;
}

/* glmFindGroup: Find a material in the model */
GLuint glmFindMaterial(GLMmodel* model, char* name)
{
	GLuint i;

	/* XXX doing a linear search on a string key'd list is pretty lame,
	but it works and is fast enough for now. */
	for (i = 0; i < model->nummaterials; i++) {
		if (!strcmp(model->materials[i].name, name))
			goto found;
	}

	/* didn't find the name, so print a warning and return the default
	material (0). */
	i = 0;

found:
	return i;
}


/* glmDirName: return the directory given a path
*
* path - filesystem path
*
* NOTE: the return value should be free'd.
*/
static char* glmDirName(const char* path)
{
	char* dir;
	char s[256];
	int i;

	dir = STRDUP(path);

	i = (int)strlen(dir);
#ifdef _WIN32
	while (dir[i+1]!=GETSLASH() &&
		   dir[i+1]!=GETSLASHALT() && i>0)
		i--;
#endif
#ifdef _DARWIN
	while (dir[i + 1] != GETSLASH() && i>0)
		i--;
#endif

	STRNCPY(s, sizeof(s), dir, i+2);

	s[i+2]='\0';

	dir = STRDUP(s);
	return dir;
}


/* glmReadMTL: read a wavefront material library file
*
* model - properly initialized GLMmodel structure
* name  - name of the material library
*/
static bool glmReadMTL(GLMmodel* model, char* name)
{
	FILE* file;
	//char* dir;
	//char* filename;
	char buf[128];
	char line[256];
	GLuint nummaterials, i;

	//dir = glmDirName(model->pathname);
	//int size_fn = int(strlen(dir) + strlen(name) + 1);
	//filename = (char*)malloc(sizeof(char) * size_fn);
	//STRCPY(filename, size_fn, dir);
	//STRCAT(filename, size_fn, name);
	//free(dir);
	std::string dir(model->pathname);
	std::wstring wdir = s2ws(dir);
	wdir = GET_PATH(wdir);
	std::string str(name);
	std::wstring wname = s2ws(str);
	std::wstring filename = wdir + wname;
	str = ws2s(filename);

	if (!FOPEN(&file, str, "rb"))
	{
		//free(filename);
		return false;
	}
	//free(filename);

	//find file size
	fseek(file, 0L, SEEK_END);
	long size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	//read file to a string
	char* mtl_content = (char*)malloc(size+1);
	if (!fread(mtl_content, sizeof(char), size, file))
	{
		fclose(file);
		return false;
	}
	mtl_content[size] = 0;
	/* close the file */
	fclose(file);

	/* count the number of materials in the file */
	nummaterials = 1;
	char *mem_file = mtl_content;
	while(sgets(line, sizeof(line), &mem_file))
	{
		if (!SSCANF(line, "%s", buf))
			break;

		switch(buf[0])
		{
		case '#':               /* comment */
			break;
		case 'n':               /* newmtl */
			nummaterials++;
			break;
		default:
			break;
		}
	}

	//allocate
	model->materials = (GLMmaterial*)malloc(sizeof(GLMmaterial) * nummaterials);
	model->nummaterials = nummaterials;

	/* set the default material */
	for (i = 0; i < nummaterials; i++)
	{
		model->materials[i].name = NULL;
		model->materials[i].shininess = 30.0f;
		model->materials[i].diffuse[0] = 0.7f;
		model->materials[i].diffuse[1] = 0.7f;
		model->materials[i].diffuse[2] = 0.7f;
		model->materials[i].diffuse[3] = 1.0f;
		model->materials[i].ambient[0] = 0.0f;
		model->materials[i].ambient[1] = 0.0f;
		model->materials[i].ambient[2] = 0.0f;
		model->materials[i].ambient[3] = 1.0f;
		model->materials[i].specular[0] = 0.0f;
		model->materials[i].specular[1] = 0.0f;
		model->materials[i].specular[2] = 0.0f;
		model->materials[i].specular[3] = 1.0f;
		model->materials[i].emmissive[0] = 0.0f;
		model->materials[i].emmissive[1] = 0.0f;
		model->materials[i].emmissive[2] = 0.0f;
		model->materials[i].emmissive[3] = 0.0f;
		model->materials[i].havetexture = GL_FALSE;
		model->materials[i].textureID = 0;
	}
	model->materials[0].name = STRDUP("default");

	/* now, read in the data */
	nummaterials = 0;
	mem_file = mtl_content;
	while(sgets(line, sizeof(line), &mem_file))
	{
		if (!SSCANF(line, "%s", buf))
			break;

		switch(buf[0])
		{
		case '#':               /* comment */
			break;
		case 'n':               /* newmtl */
			{
				char mtlname[128];
				SSCANF(line, "%s %s", mtlname, mtlname);
				nummaterials++;
				model->materials[nummaterials].name = STRDUP(mtlname);
			}
			break;
		case 'N':
			if (buf[1] == 's')
			{
				SSCANF(line, "%s %f", buf, 
					&model->materials[nummaterials].shininess);
				/* wavefront shininess is from [0, 100], so scale for OpenGL */
				model->materials[nummaterials].shininess /= 0.9f;
			}
			break;
		case 'm':
			{
				char texname[128];
				SSCANF(line, "%s %s", texname,  texname);
				//dir = glmDirName(model->pathname);
				//int size_tn = int(strlen(dir) + strlen(texname) + 1);
				//filename = (char*)malloc(sizeof(char) * size_tn);
				//STRCPY(filename, size_tn, dir);
				//STRCAT(filename, size_tn, texname);
				//free(dir);
				std::string str(texname);
				std::wstring wname = s2ws(str);
				std::wstring filename = wdir + wname;
				str = ws2s(filename);

				TextureImage texture;
				if (glmLoadTGA(&texture, str.c_str(),
					&model->materials[nummaterials].textureID))
				{
					model->materials[nummaterials].havetexture = GL_TRUE;
					free(texture.imageData);
					model->hastexture = GL_TRUE;
				}
				//free(filename);
			}
			break;
		case 'K':
			switch(buf[1])
			{
			case 'd':
				SSCANF(line, "%s %f %f %f", buf, 
					&model->materials[nummaterials].diffuse[0],
					&model->materials[nummaterials].diffuse[1],
					&model->materials[nummaterials].diffuse[2]);
				if (model->materials[nummaterials].diffuse[0]==0.0f &&
					model->materials[nummaterials].diffuse[1]==0.0f &&
					model->materials[nummaterials].diffuse[2]==0.0f)
				{
					model->materials[nummaterials].diffuse[0] = 1.0f;
					model->materials[nummaterials].diffuse[1] = 1.0f;
					model->materials[nummaterials].diffuse[2] = 1.0f;
				}
				break;
			case 's':
				SSCANF(line, "%s %f %f %f", buf, 
					&model->materials[nummaterials].specular[0],
					&model->materials[nummaterials].specular[1],
					&model->materials[nummaterials].specular[2]);
				break;
			case 'a':
				SSCANF(line, "%s %f %f %f", buf,
					&model->materials[nummaterials].ambient[0],
					&model->materials[nummaterials].ambient[1],
					&model->materials[nummaterials].ambient[2]);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}

	free(mtl_content);
	return true;
}

/* glmWriteMTL: write a wavefront material library file
*
* model   - properly initialized GLMmodel structure
* modelpath  - pathname of the model being written
* mtllibname - name of the material library to be written
*/
static GLvoid glmWriteMTL(GLMmodel* model, const char* modelpath, char* mtllibname)
{
	FILE* file;
	char* dir;
	char* filename;
	GLMmaterial* material;
	GLuint i;

	dir = glmDirName(modelpath);
	size_t filename_len = strlen(dir)+strlen(mtllibname);
	filename = (char*)malloc(sizeof(char) * filename_len);
	STRCPY(filename, filename_len, dir);
	STRCAT(filename, filename_len, mtllibname);
	free(dir);

	/* open the file */
	std::string str = filename;
	if (!FOPEN(&file, str, "wt"))
		return;
	free(filename);

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront MTL generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library\n");
	fprintf(file, "#  Nate Robins\n");
	fprintf(file, "#  ndr@pobox.com\n");
	fprintf(file, "#  http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n\n");

	for (i = 0; i < model->nummaterials; i++) {
		material = &model->materials[i];
		fprintf(file, "newmtl %s\n", material->name);
		fprintf(file, "Ka %f %f %f\n",
			material->ambient[0], material->ambient[1], material->ambient[2]);
		fprintf(file, "Kd %f %f %f\n",
			material->diffuse[0], material->diffuse[1], material->diffuse[2]);
		fprintf(file, "Ks %f %f %f\n",
			material->specular[0],material->specular[1],material->specular[2]);
		fprintf(file, "Ns %f\n", material->shininess / 128.0 * 1000.0);
		fprintf(file, "\n");
	}
}

//help function to read string
char *sgets( char * str, int num, char **input )
{
	char *next = *input;
	int  numread = 0;

	while ( numread + 1 < num && *next ) {
		int isnewline = ( *next == '\n' );
		*str++ = *next++;
		numread++;
		// newline terminates the line but is included
		if ( isnewline )
			break;
	}

	if ( numread == 0 )
		return NULL;  // "eof"

	// must have hit the null terminator or end of line
	*str = '\0';  // null terminate this tring
	// set up input for next call
	*input = next;
	return str;
}

/* glmFirstPass: first pass at a Wavefront OBJ file that gets all the
* statistics of the model (such as #vertices, #normals, etc)
*
* model - properly initialized GLMmodel structure
* file  - (fopen'd) file descriptor
*/
static GLboolean glmFirstPass(GLMmodel* model, char* file)
{
	GLuint numvertices;        /* number of vertices in model */
	GLuint numnormals;         /* number of normals in model */
	GLuint numtexcoords;       /* number of texcoords in model */
	GLuint numcolors;          /* number of colors in model */
	GLuint numlines;           /* number of lines in model */
	GLuint numtriangles;       /* number of triangles in model */
	GLMgroup* group;           /* current group */
	char buf[128];
	char line[2048];
	bool no_fail = true;

	/* make a default group */
	char  tmp[] = "default";
	group = glmAddGroup(model, tmp);

	numvertices = numnormals = numtexcoords = numcolors = numlines = numtriangles = 0;

	while (sgets(line, sizeof(line), &file))
	{
		if (!SSCANF(line, "%s", buf))
			break;

		switch(buf[0])
		{
		case '#':               /* comment */
			break;
		case 'v':               /* v, vn, vt */
			switch(buf[1])
			{
			case '\0':          /* vertex */
				numvertices++;
				break;
			case 'n':           /* normal */
				numnormals++;
				break;
			case 't':           /* texcoord */
				numtexcoords++;
				break;
			case 'c':           /* color */
				numcolors++;
				break;
			default:
				break;
			}
			break;
		case 'm':
			{
				char mtlname[128];
				SSCANF(line, "%s %s", mtlname, mtlname);
				model->mtllibname = STRDUP(mtlname);
				no_fail &= glmReadMTL(model, mtlname);
			}
			break;
		case 'u':
			break;
		case 'g':               /* group */
			SSCANF(line, "%s %s", buf, buf);
			group = glmAddGroup(model, buf);
			break;
		case 'l':               /* line */
			numlines++;
			break;
		case 'f':               /* face */
			{
				int count = 0;
				bool prev_is_blank = true;
				for (int i=0; i<(int)strlen(line); i++)
				{
					if (line[i] == ' ' || line[i] == '\n')
						prev_is_blank = true;
					else
					{
						if (prev_is_blank)
							count++;
						prev_is_blank = false;
					}
				}
				if (count > 3)
				{
					numtriangles += count-3;
					group->numtriangles += count-3;
				}
			}
			break;
		default:
			break;
		}
	}

	/* set the stats in the model structure */
	model->numvertices  = numvertices;
	model->numnormals   = numnormals;
	model->numtexcoords = numtexcoords;
	model->numcolors = numcolors;
	model->numlines = numlines;
	model->numtriangles = numtriangles;

	/* allocate memory for the triangles in each group */
	group = model->groups;
	while(group)
	{
		if (group->numtriangles)
			group->triangles = (GLuint*)malloc(sizeof(GLuint) * group->numtriangles);
		group->numtriangles = 0;
		group = group->next;
	}
	return no_fail;
}

/* glmSecondPass: second pass at a Wavefront OBJ file that gets all
* the data.
*
* model - properly initialized GLMmodel structure
* file  - (fopen'd) file descriptor
*/
static GLvoid glmSecondPass(GLMmodel* model, char* file)
{
	GLuint numvertices;        /* number of vertices in model */
	GLuint numnormals;         /* number of normals in model */
	GLuint numtexcoords;       /* number of texcoords in model */
	GLuint numcolors;          /* number of colors in model */
	GLuint numlines;           /* number of lines in model */
	GLuint numtriangles;       /* number of triangles in model */
	GLfloat* vertices;         /* array of vertices  */
	GLfloat* normals;          /* array of normals */
	GLfloat* texcoords;        /* array of texture coordinates */
	GLfloat* colors;           /* array of colors */
	GLMgroup* group;           /* current group pointer */
	GLuint material;           /* current material */
	int v, n, t, c;
	char buf[128];
	char line[2048];

	/* set the pointer shortcuts */
	vertices       = model->vertices;
	normals    = model->normals;
	texcoords    = model->texcoords;
	colors = model->colors;
	group      = model->groups;

	/* on the second pass through the file, read all the data into the
	allocated arrays */
	numvertices = numnormals = numtexcoords = numcolors = 1;
	numtriangles = 0;
	numlines = 0;
	material = 0;

	while (sgets(line, sizeof(line), &file))
	{
		if (!SSCANF(line, "%s", buf ))
			break;

		switch(buf[0])
		{
		case '#':               /* comment */
			break;
		case 'v':               /* v, vn, vt */
			switch(buf[1])
			{
			case '\0':          /* vertex */
				SSCANF(line+2, "%f %f %f",
					&vertices[3 * numvertices + 0],
					&vertices[3 * numvertices + 1],
					&vertices[3 * numvertices + 2]);
				numvertices++;
				break;
			case 'n':           /* normal */
				SSCANF(line+3, "%f %f %f",
					&normals[3 * numnormals + 0],
					&normals[3 * numnormals + 1],
					&normals[3 * numnormals + 2]);
				numnormals++;
				break;
			case 't':           /* texcoord */
				SSCANF(line+3, "%f %f",
					&texcoords[2 * numtexcoords + 0],
					&texcoords[2 * numtexcoords + 1]);
				numtexcoords++;
				break;
			case 'c':           /* color */
				SSCANF(line + 3, "%f %f %f %f",
					&colors[4 * numcolors + 0],
					&colors[4 * numcolors + 1],
					&colors[4 * numcolors + 2],
					&colors[4 * numcolors + 3]);
				numcolors++;
				break;
			}
			break;
		case 'u':
			{
				char mtlname[128];
				SSCANF(line, "%s %s", buf,
					mtlname);
				group->material = material = glmFindMaterial(model, mtlname);
			}
			break;
		case 'g':               /* group */
			SSCANF(line, "%s %s", buf,
				buf);
			group = glmFindGroup(model, buf);
			group->material = material;
			break;
		case 'l':               /* line */
			{
				int count = 0;
				bool prev_is_blank = true;
				for (int i=0; i<(int)strlen(line); i++)
				{
					if (line[i] == ' ' || line[i] == '\n' || line[i] == '\r')
						prev_is_blank = true;
					else
					{
						if (prev_is_blank)
							count++;
						prev_is_blank = false;
					}
				}
				//alocate
				L(numlines).numvertices = count - 1;
				L(numlines).vindices = (GLuint*)malloc(sizeof(GLuint) *
					L(numlines).numvertices);
				//read again
				count = 0;
				int buf_count = 0;
				prev_is_blank = true;
				for (int i=0; i<(int)strlen(line); i++)
				{
					if (line[i] == ' ' || line[i] == '\n' || line[i] == '\r')
					{
						prev_is_blank = true;
						buf[buf_count] = 0;
						buf_count = 0;

						//add buf
						if (count > 1)
						{
							if (SSCANF(buf, "%d", &v) == 1)
								L(numlines).vindices[count-2] = v < 0 ? v + numvertices : v;
						}
					}
					else
					{
						if (prev_is_blank)
							count++;
						prev_is_blank = false;
						buf[buf_count] = line[i];
						buf_count++;
					}
				}
			}
			numlines++;
			break;
		case 'f': /* face */
		{
			v = n = t = c = 0;

			int count = 0;
			int buf_count = 0;
			bool prev_is_blank = true;
			for (int i = 0; i < (int)strlen(line); i++)
			{
				if (line[i] == ' ' || line[i] == '\n')
				{
					prev_is_blank = true;
					buf[buf_count] = 0;
					buf_count = 0;

					if (count > 1 && count < 5)
					{
						if (strstr(buf, "//"))
						{
							if (SSCANF(buf, "%d//%d/%d", &v, &n, &c) == 3)
							{
								T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
								T(numtriangles).nindices[count - 2] = n < 0 ? n + numnormals : n;
								T(numtriangles).cindices[count - 2] = c < 0 ? c + numcolors : c;
							}
							else
							{
								SSCANF(buf, "%d//%d", &v, &n);
								T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
								T(numtriangles).nindices[count - 2] = n < 0 ? n + numnormals : n;
							}
						}
						else if (SSCANF(buf, "%d/%d/%d/%d", &v, &t, &n, &c) == 4)
						{
							T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
							T(numtriangles).tindices[count - 2] = t < 0 ? t + numtexcoords : t;
							T(numtriangles).nindices[count - 2] = n < 0 ? n + numnormals : n;
							T(numtriangles).cindices[count - 2] = c < 0 ? c + numcolors : c;
						}
						else if (SSCANF(buf, "%d/%d/%d", &v, &t, &n) == 3)
						{
							T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
							T(numtriangles).tindices[count - 2] = t < 0 ? t + numtexcoords : t;
							T(numtriangles).nindices[count - 2] = n < 0 ? n + numnormals : n;
						}
						else if (SSCANF(buf, "%d/%d/%d", &v, &t, &c) == 3)
						{
							T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
							T(numtriangles).tindices[count - 2] = t < 0 ? t + numtexcoords : t;
							T(numtriangles).cindices[count - 2] = c < 0 ? c + numcolors : c;
						}
						else if (SSCANF(buf, "%d/%d", &v, &t) == 2)
						{
							T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
							T(numtriangles).tindices[count - 2] = t < 0 ? t + numtexcoords : t;
						}
						else if (SSCANF(buf, "%d/%d", &v, &c) == 2)
						{
							T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
							T(numtriangles).cindices[count - 2] = c < 0 ? c + numcolors : c;
						}
						else
						{
							SSCANF(buf, "%d", &v);
							T(numtriangles).vindices[count - 2] = v < 0 ? v + numvertices : v;
						}

						if (count == 4)
						{
							group->triangles[group->numtriangles++] = numtriangles;
							numtriangles++;
						}
					}
					else if (count > 4)
					{
						// Fan triangulation logic (same as before), extended to handle cindices
						SSCANF(buf, "%d/%d/%d/%d", &v, &t, &n, &c);
						T(numtriangles).vindices[0] = T(numtriangles - 1).vindices[0];
						T(numtriangles).tindices[0] = T(numtriangles - 1).tindices[0];
						T(numtriangles).nindices[0] = T(numtriangles - 1).nindices[0];
						T(numtriangles).cindices[0] = T(numtriangles - 1).cindices[0];

						T(numtriangles).vindices[1] = T(numtriangles - 1).vindices[2];
						T(numtriangles).tindices[1] = T(numtriangles - 1).tindices[2];
						T(numtriangles).nindices[1] = T(numtriangles - 1).nindices[2];
						T(numtriangles).cindices[1] = T(numtriangles - 1).cindices[2];

						T(numtriangles).vindices[2] = v < 0 ? v + numvertices : v;
						T(numtriangles).tindices[2] = t < 0 ? t + numtexcoords : t;
						T(numtriangles).nindices[2] = n < 0 ? n + numnormals : n;
						T(numtriangles).cindices[2] = c < 0 ? c + numcolors : c;

						group->triangles[group->numtriangles++] = numtriangles;
						numtriangles++;
					}
				}
				else
				{
					if (prev_is_blank)
						count++;
					prev_is_blank = false;
					buf[buf_count] = line[i];
					buf_count++;
				}
			}
		}
		break;
		default:
			break;
		}
	}
}


/* public functions */
/* glmUnitize: "unitize" a model by translating it to the origin and
* scaling it to fit in a unit cube around the origin.   Returns the
* scalefactor used.
*
* model - properly initialized GLMmodel structure
*/
GLfloat glmUnitize(GLMmodel* model)
{
	GLuint i;
	GLfloat maxx, minx, maxy, miny, maxz, minz;
	GLfloat cx, cy, cz, w, h, d;
	GLfloat scale;

	assert(model);
	assert(model->vertices);

	/* get the max/mins */
	maxx = minx = model->vertices[3 + 0];
	maxy = miny = model->vertices[3 + 1];
	maxz = minz = model->vertices[3 + 2];
	for (i = 1; i <= model->numvertices; i++) {
		if (maxx < model->vertices[3 * i + 0])
			maxx = model->vertices[3 * i + 0];
		if (minx > model->vertices[3 * i + 0])
			minx = model->vertices[3 * i + 0];

		if (maxy < model->vertices[3 * i + 1])
			maxy = model->vertices[3 * i + 1];
		if (miny > model->vertices[3 * i + 1])
			miny = model->vertices[3 * i + 1];

		if (maxz < model->vertices[3 * i + 2])
			maxz = model->vertices[3 * i + 2];
		if (minz > model->vertices[3 * i + 2])
			minz = model->vertices[3 * i + 2];
	}

	/* calculate model width, height, and depth */
	w = glmAbs(maxx) + glmAbs(minx);
	h = glmAbs(maxy) + glmAbs(miny);
	d = glmAbs(maxz) + glmAbs(minz);

	/* calculate center of the model */
	cx = (maxx + minx) / 2.0f;
	cy = (maxy + miny) / 2.0f;
	cz = (maxz + minz) / 2.0f;

	/* calculate unitizing scale factor */
	scale = 2.0f / glmMax(glmMax(w, h), d);

	/* translate around center then scale */
	for (i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] -= cx;
		model->vertices[3 * i + 1] -= cy;
		model->vertices[3 * i + 2] -= cz;
		model->vertices[3 * i + 0] *= scale;
		model->vertices[3 * i + 1] *= scale;
		model->vertices[3 * i + 2] *= scale;
	}

	return scale;
}

/* glmBoundingBox: Calculates the bounding box, which is an array of six
* floats (minx, maxx, miny, maxy, minz, maxz) of a model.
* Written by Scott D. Anderson, based on Nate Robbins's original
* glmDimensions function, then re-wrote glmDimensions to use this.
*
* model   - initialized GLMmodel structure
* boundingbox - array of 6 GLfloats (GLfloat boundingbox[6])
*/
GLvoid glmBoundingBox(GLMmodel* model, GLfloat* boundingbox)
{
	GLuint i;
	GLfloat maxx, minx, maxy, miny, maxz, minz;

	assert(model);
	assert(model->vertices);
	assert(boundingbox);

	/* get the max/mins */
	maxx = minx = model->vertices[3 + 0];
	maxy = miny = model->vertices[3 + 1];
	maxz = minz = model->vertices[3 + 2];
	for (i = 1; i < model->numvertices; i++) {
		if (maxx < model->vertices[3 * i + 0])
			maxx = model->vertices[3 * i + 0];
		if (minx > model->vertices[3 * i + 0])
			minx = model->vertices[3 * i + 0];

		if (maxy < model->vertices[3 * i + 1])
			maxy = model->vertices[3 * i + 1];
		if (miny > model->vertices[3 * i + 1])
			miny = model->vertices[3 * i + 1];

		if (maxz < model->vertices[3 * i + 2])
			maxz = model->vertices[3 * i + 2];
		if (minz > model->vertices[3 * i + 2])
			minz = model->vertices[3 * i + 2];
	}

	/* calculate model width, height, and depth */
	boundingbox[0] = minx;
	boundingbox[1] = maxx;
	boundingbox[2] = miny;
	boundingbox[3] = maxy;
	boundingbox[4] = minz;
	boundingbox[5] = maxz;
}

//get the center
GLvoid glmCenter(GLMmodel* model, GLfloat* center)
{
	GLfloat boundingbox[6];
	glmBoundingBox(model, boundingbox);
	center[0] = (boundingbox[0] + boundingbox[1]) / 2.0f;
	center[1] = (boundingbox[2] + boundingbox[3]) / 2.0f;
	center[2] = (boundingbox[4] + boundingbox[5]) / 2.0f;
}

/* glmDimensions: Calculates the dimensions (width, height, depth) of
* a model.
*
* model   - initialized GLMmodel structure
* dimensions - array of 3 GLfloats (GLfloat dimensions[3])
*/
GLvoid glmDimensions(GLMmodel* model, GLfloat* dimensions)
{
	assert(model);
	assert(model->vertices);
	assert(dimensions);

	GLfloat bbox[6];
	glmBoundingBox(model,bbox);

	// Computation changed by Scott D. Anderson
	dimensions[0] = glmAbs(bbox[1]-bbox[0]);
	dimensions[1] = glmAbs(bbox[3]-bbox[2]);
	dimensions[2] = glmAbs(bbox[5]-bbox[4]);
}

/* glmArea: Calculates the sum of face areas of a model
*
* model      - initialized GLMmodel structure
* area       - single GLfloat
*/
GLvoid glmArea(GLMmodel* model, GLfloat* scale, GLfloat* area)
{
	assert(model);
	assert(model->vertices);

	GLfloat result = 0.0f;
	GLfloat u[3];
	GLfloat v[3];
	GLfloat w[3];
	GLuint i;
	for (i = 0; i < model->numtriangles; i++) {
		model->triangles[i].findex = i + 1;

		u[0] = (model->vertices[3 * T(i).vindices[1] + 0] -
			model->vertices[3 * T(i).vindices[0] + 0]) *
			scale[0];
		u[1] = (model->vertices[3 * T(i).vindices[1] + 1] -
			model->vertices[3 * T(i).vindices[0] + 1]) *
			scale[1];
		u[2] = (model->vertices[3 * T(i).vindices[1] + 2] -
			model->vertices[3 * T(i).vindices[0] + 2]) *
			scale[2];

		v[0] = (model->vertices[3 * T(i).vindices[2] + 0] -
			model->vertices[3 * T(i).vindices[0] + 0]) *
			scale[0];
		v[1] = (model->vertices[3 * T(i).vindices[2] + 1] -
			model->vertices[3 * T(i).vindices[0] + 1]) *
			scale[1];
		v[2] = (model->vertices[3 * T(i).vindices[2] + 2] -
			model->vertices[3 * T(i).vindices[0] + 2]) *
			scale[2];

		glmCross(u, v, w);
		result += (GLfloat)sqrt(w[0] * w[0] + w[1] * w[1] + w[2] * w[2])*0.5f;
	}
	*area = result;
}

/* Scale, translate and rotate the model
* Similar to OpenGL calls
*/
GLvoid glmScalef(GLMmodel* model, GLfloat x, GLfloat y, GLfloat z)
{
	GLuint i;

	for (i=1 ; i<=model->numvertices ; i++)
	{
		model->vertices[3 * i + 0] *= x;
		model->vertices[3 * i + 1] *= y;
		model->vertices[3 * i + 2] *= z;
	}
}

GLvoid glmTranslatef(GLMmodel* model, GLfloat x, GLfloat y, GLfloat z)
{
	GLuint i;

	for (i=1 ; i<=model->numvertices ; i++)
	{
		model->vertices[3 * i + 0] += x;
		model->vertices[3 * i + 1] += y;
		model->vertices[3 * i + 2] += z;
	}
}

//Rotate around the vector going throught the center of boundingbox
GLvoid glmRotatef(GLMmodel* model, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
	double c = cos(angle);
	double s = sin(angle);
	double d = sqrt(x*x + y*y + z*z);
	double dx = x / d;
	double dy = y / d;
	double dz = z / d;
	double tx, ty, tz;

	GLfloat center[3];
	glmCenter(model, center);

	GLuint i;
	double mat[9] =
	{dx*dx*(1-c)+c,    dx*dy*(1-c)-dz*s, dx*dz*(1-c)+dy*s,
	dy*dx*(1-c)+dz*s, dy*dy*(1-c)+c,    dy*dz*(1-c)-dx*s,
	dx*dz*(1-c)-dy*s, dy*dz*(1-c)+dx*s, dz*dz*(1-c)+c};
	for (i=1 ; i<=model->numvertices ; i++)
	{
		tx = model->vertices[3 * i + 0] - center[0];
		ty = model->vertices[3 * i + 1] - center[1];
		tz = model->vertices[3 * i + 2] - center[2];
		model->vertices[3 * i + 0] = GLfloat(
			mat[0]*tx +
			mat[1]*ty +
			mat[2]*tz);
		model->vertices[3 * i + 1] = GLfloat(
			mat[3]*tx +
			mat[4]*ty +
			mat[5]*tz);
		model->vertices[3 * i + 2] = GLfloat(
			mat[6]*tx +
			mat[7]*ty +
			mat[8]*tz);
		model->vertices[3 * i + 0] += center[0];
		model->vertices[3 * i + 1] += center[1];
		model->vertices[3 * i + 2] += center[2];
	}
}

/* glmScale: Scales a model by a given amount.
*
* model - properly initialized GLMmodel structure
* scale - scalefactor (0.5 = half as large, 2.0 = twice as large)
*/
GLvoid glmScale(GLMmodel* model, GLfloat scale)
{
	GLuint i;

	for (i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] *= scale;
		model->vertices[3 * i + 1] *= scale;
		model->vertices[3 * i + 2] *= scale;
	}
}

/* glmReverseWinding: Reverse the polygon winding for all polygons in
* this model.   Default winding is counter-clockwise.  Also changes
* the direction of the normals.
*
* model - properly initialized GLMmodel structure
*/
GLvoid glmReverseWinding(GLMmodel* model)
{
	GLuint i, swap;

	assert(model);

	for (i = 0; i < model->numtriangles; i++) {
		swap = T(i).vindices[0];
		T(i).vindices[0] = T(i).vindices[2];
		T(i).vindices[2] = swap;

		if (model->numnormals) {
			swap = T(i).nindices[0];
			T(i).nindices[0] = T(i).nindices[2];
			T(i).nindices[2] = swap;
		}

		if (model->numtexcoords) {
			swap = T(i).tindices[0];
			T(i).tindices[0] = T(i).tindices[2];
			T(i).tindices[2] = swap;
		}

		if (model->numcolors) {
			swap = T(i).cindices[0];
			T(i).cindices[0] = T(i).cindices[2];
			T(i).cindices[2] = swap;
		}
	}

	/* reverse facet normals */
	for (i = 1; i <= model->numfacetnorms; i++) {
		model->facetnorms[3 * i + 0] = -model->facetnorms[3 * i + 0];
		model->facetnorms[3 * i + 1] = -model->facetnorms[3 * i + 1];
		model->facetnorms[3 * i + 2] = -model->facetnorms[3 * i + 2];
	}

	/* reverse vertex normals */
	for (i = 1; i <= model->numnormals; i++) {
		model->normals[3 * i + 0] = -model->normals[3 * i + 0];
		model->normals[3 * i + 1] = -model->normals[3 * i + 1];
		model->normals[3 * i + 2] = -model->normals[3 * i + 2];
	}
}

/* glmFacetNormals: Generates facet normals for a model (by taking the
* cross product of the two vectors derived from the sides of each
* triangle).  Assumes a counter-clockwise winding.
*
* model - initialized GLMmodel structure
*/
GLvoid glmFacetNormals(GLMmodel* model)
{
	GLuint  i;
	GLfloat u[3];
	GLfloat v[3];

	assert(model);
	assert(model->vertices);

	/* clobber any old facetnormals */
	if (model->facetnorms)
		free(model->facetnorms);

	/* allocate memory for the new facet normals */
	model->numfacetnorms = model->numtriangles;
	model->facetnorms = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numfacetnorms + 1));

	for (i = 0; i < model->numtriangles; i++) {
		model->triangles[i].findex = i+1;

		u[0] = model->vertices[3 * T(i).vindices[1] + 0] -
			model->vertices[3 * T(i).vindices[0] + 0];
		u[1] = model->vertices[3 * T(i).vindices[1] + 1] -
			model->vertices[3 * T(i).vindices[0] + 1];
		u[2] = model->vertices[3 * T(i).vindices[1] + 2] -
			model->vertices[3 * T(i).vindices[0] + 2];

		v[0] = model->vertices[3 * T(i).vindices[2] + 0] -
			model->vertices[3 * T(i).vindices[0] + 0];
		v[1] = model->vertices[3 * T(i).vindices[2] + 1] -
			model->vertices[3 * T(i).vindices[0] + 1];
		v[2] = model->vertices[3 * T(i).vindices[2] + 2] -
			model->vertices[3 * T(i).vindices[0] + 2];

		glmCross(u, v, &model->facetnorms[3 * (i+1)]);
		glmNormalize(&model->facetnorms[3 * (i+1)]);
	}
}

/* glmVertexNormals: Generates smooth vertex normals for a model.
* First builds a list of all the triangles each vertex is in.   Then
* loops through each vertex in the the list averaging all the facet
* normals of the triangles each vertex is in.   Finally, sets the
* normal index in the triangle for the vertex to the generated smooth
* normal.   If the dot product of a facet normal and the facet normal
* associated with the first triangle in the list of triangles the
* current vertex is in is greater than the cosine of the angle
* parameter to the function, that facet normal is not added into the
* average normal calculation and the corresponding vertex is given
* the facet normal.  This tends to preserve hard edges.  The angle to
* use depends on the model, but 90 degrees is usually a good start.
*
* model - initialized GLMmodel structure
* angle - maximum angle (in degrees) to smooth across
*/
GLvoid glmVertexNormals(GLMmodel* model, GLfloat angle)
{
	GLMnode* node;
	GLMnode* tail;
	GLMnode** members;
	GLfloat* normals;
	GLuint numnormals;
	GLfloat average[3];
	GLfloat dot, cos_angle;
	GLuint i, avg;

	assert(model);
	assert(model->facetnorms);

	/* calculate the cosine of the angle (in degrees) */
	cos_angle = cos(angle * M_PI / 180.0f);

	/* nuke any previous normals */
	if (model->normals)
		free(model->normals);

	/* allocate space for new normals */
	model->numnormals = model->numtriangles * 3; /* 3 normals per triangle */
	model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));

	/* allocate a structure that will hold a linked list of triangle
	indices for each vertex */
	members = (GLMnode**)malloc(sizeof(GLMnode*) * (model->numvertices + 1));
	for (i = 1; i <= model->numvertices; i++)
		members[i] = NULL;

	/* for every triangle, create a node for each vertex in it */
	for (i = 0; i < model->numtriangles; i++) {
		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[T(i).vindices[0]];
		members[T(i).vindices[0]] = node;

		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[T(i).vindices[1]];
		members[T(i).vindices[1]] = node;

		node = (GLMnode*)malloc(sizeof(GLMnode));
		node->index = i;
		node->next  = members[T(i).vindices[2]];
		members[T(i).vindices[2]] = node;
	}

	/* calculate the average normal for each vertex */
	numnormals = 1;
	for (i = 1; i <= model->numvertices; i++) {
		/* calculate an average normal for this vertex by averaging the
		facet normal of every triangle this vertex is in */
		node = members[i];
		if (!node)
			fprintf(stderr, "glmVertexNormals(): vertex w/o a triangle\n");
		average[0] = 0.0; average[1] = 0.0; average[2] = 0.0;
		avg = 0;
		while (node) {
			/* only average if the dot product of the angle between the two
			facet normals is greater than the cosine of the threshold
			angle -- or, said another way, the angle between the two
			facet normals is less than (or equal to) the threshold angle */
			dot = glmDot(&model->facetnorms[3 * T(node->index).findex],
				&model->facetnorms[3 * T(members[i]->index).findex]);
			if (dot > cos_angle) {
				node->averaged = GL_TRUE;
				average[0] += model->facetnorms[3 * T(node->index).findex + 0];
				average[1] += model->facetnorms[3 * T(node->index).findex + 1];
				average[2] += model->facetnorms[3 * T(node->index).findex + 2];
				avg = 1;            /* we averaged at least one normal! */
			} else {
				node->averaged = GL_FALSE;
			}
			node = node->next;
		}

		if (avg) {
			/* normalize the averaged normal */
			glmNormalize(average);

			/* add the normal to the vertex normals list */
			model->normals[3 * numnormals + 0] = average[0];
			model->normals[3 * numnormals + 1] = average[1];
			model->normals[3 * numnormals + 2] = average[2];
			avg = numnormals;
			numnormals++;
		}

		/* set the normal of this vertex in each triangle it is in */
		node = members[i];
		while (node) {
			if (node->averaged) {
				/* if this node was averaged, use the average normal */
				if (T(node->index).vindices[0] == i)
					T(node->index).nindices[0] = avg;
				else if (T(node->index).vindices[1] == i)
					T(node->index).nindices[1] = avg;
				else if (T(node->index).vindices[2] == i)
					T(node->index).nindices[2] = avg;
			} else {
				/* if this node wasn't averaged, use the facet normal */
				model->normals[3 * numnormals + 0] =
					model->facetnorms[3 * T(node->index).findex + 0];
				model->normals[3 * numnormals + 1] =
					model->facetnorms[3 * T(node->index).findex + 1];
				model->normals[3 * numnormals + 2] =
					model->facetnorms[3 * T(node->index).findex + 2];
				if (T(node->index).vindices[0] == i)
					T(node->index).nindices[0] = numnormals;
				else if (T(node->index).vindices[1] == i)
					T(node->index).nindices[1] = numnormals;
				else if (T(node->index).vindices[2] == i)
					T(node->index).nindices[2] = numnormals;
				numnormals++;
			}
			node = node->next;
		}
	}

	model->numnormals = numnormals - 1;

	/* free the member information */
	for (i = 1; i <= model->numvertices; i++) {
		node = members[i];
		while (node) {
			tail = node;
			node = node->next;
			free(tail);
		}
	}
	free(members);

	/* pack the normals array (we previously allocated the maximum
	number of normals that could possibly be created (numtriangles *
	3), so get rid of some of them (usually alot unless none of the
	facet normals were averaged)) */
	normals = model->normals;
	model->normals = (GLfloat*)malloc(sizeof(GLfloat)* 3* (model->numnormals+1));
	for (i = 1; i <= model->numnormals; i++) {
		model->normals[3 * i + 0] = normals[3 * i + 0];
		model->normals[3 * i + 1] = normals[3 * i + 1];
		model->normals[3 * i + 2] = normals[3 * i + 2];
	}
	free(normals);
}


/* glmLinearTexture: Generates texture coordinates according to a
* linear projection of the texture map.  It generates these by
* linearly mapping the vertices onto a square.
*
* model - pointer to initialized GLMmodel structure
*/
GLvoid glmLinearTexture(GLMmodel* model)
{
	GLMgroup *group;
	GLfloat dimensions[3];
	GLfloat x, y, scalefactor;
	GLuint i;

	assert(model);

	if (model->texcoords)
		free(model->texcoords);
	model->numtexcoords = model->numvertices;
	model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));

	glmDimensions(model, dimensions);
	scalefactor = 2.0f /
		glmAbs(glmMax(glmMax(dimensions[0], dimensions[1]), dimensions[2]));

	/* do the calculations */
	for(i = 1; i <= model->numvertices; i++) {
		x = model->vertices[3 * i + 0] * scalefactor;
		y = model->vertices[3 * i + 2] * scalefactor;
		model->texcoords[2 * i + 0] = (x + 1.0f) / 2.0f;
		model->texcoords[2 * i + 1] = (y + 1.0f) / 2.0f;
	}

	/* go through and put texture coordinate indices in all the triangles */
	group = model->groups;
	while(group) {
		for(i = 0; i < group->numtriangles; i++) {
			T(group->triangles[i]).tindices[0] = T(group->triangles[i]).vindices[0];
			T(group->triangles[i]).tindices[1] = T(group->triangles[i]).vindices[1];
			T(group->triangles[i]).tindices[2] = T(group->triangles[i]).vindices[2];
		}
		group = group->next;
	}
}

/* glmSpheremapTexture: Generates texture coordinates according to a
* spherical projection of the texture map.  Sometimes referred to as
* spheremap, or reflection map texture coordinates.  It generates
* these by using the normal to calculate where that vertex would map
* onto a sphere.  Since it is impossible to map something flat
* perfectly onto something spherical, there is distortion at the
* poles.  This particular implementation causes the poles along the X
* axis to be distorted.
*
* model - pointer to initialized GLMmodel structure
*/
GLvoid glmSpheremapTexture(GLMmodel* model)
{
	GLMgroup* group;
	GLfloat theta, phi, rho, x, y, z, r;
	GLuint i;

	assert(model);
	assert(model->normals);

	if (model->texcoords)
		free(model->texcoords);
	model->numtexcoords = model->numnormals;
	model->texcoords=(GLfloat*)malloc(sizeof(GLfloat)*2*(model->numtexcoords+1));

	for (i = 1; i <= model->numnormals; i++) {
		z = model->normals[3 * i + 0];  /* re-arrange for pole distortion */
		y = model->normals[3 * i + 1];
		x = model->normals[3 * i + 2];
		r = sqrt((x * x) + (y * y));
		rho = sqrt((r * r) + (z * z));

		if(r == 0.0) {
			theta = 0.0;
			phi = 0.0;
		} else {
			if(z == 0.0)
				phi = M_PI / 2.0f;
			else
				phi = acos(z / rho);

			if(y == 0.0)
				theta = M_PI / 2.0f;
			else
				theta = asin(y / r) + (M_PI / 2.0f);
		}

		model->texcoords[2 * i + 0] = theta / M_PI;
		model->texcoords[2 * i + 1] = phi / M_PI;
	}

	/* go through and put texcoord indices in all the triangles */
	group = model->groups;
	while(group) {
		for (i = 0; i < group->numtriangles; i++) {
			T(group->triangles[i]).tindices[0] = T(group->triangles[i]).nindices[0];
			T(group->triangles[i]).tindices[1] = T(group->triangles[i]).nindices[1];
			T(group->triangles[i]).tindices[2] = T(group->triangles[i]).nindices[2];
		}
		group = group->next;
	}
}

/* glmDelete: Deletes a GLMmodel structure.
*
* model - initialized GLMmodel structure
*/
GLvoid glmDelete(GLMmodel* model)
{
	GLMgroup* group;
	GLuint i;

	assert(model);

	if (model->pathname)     free(model->pathname);
	if (model->mtllibname) free(model->mtllibname);
	if (model->vertices)     free(model->vertices);
	if (model->normals)  free(model->normals);
	if (model->texcoords)  free(model->texcoords);
	if (model->colors)  free(model->colors);
	if (model->facetnorms) free(model->facetnorms);
	if (model->lines)
	{
		for (i=0; i<model->numlines; i++)
			free(model->lines[i].vindices);
		free(model->lines);
	}
	if (model->triangles)  free(model->triangles);
	if (model->materials)
	{
		for (i = 0; i < model->nummaterials; i++)
		{
			free(model->materials[i].name);
			if (model->materials[i].havetexture &&
				glIsTexture(model->materials[i].textureID))
				glDeleteTextures(1, &model->materials[i].textureID);
		}
		free(model->materials);
	}
	while(model->groups)
	{
		group = model->groups;
		model->groups = model->groups->next;
		free(group->name);
		free(group->triangles);
		free(group);
	}

	free(model);
}

/* glmClear: clear the content of the mode without freeing itself
*
* model - initialized GLMmodel structure
*/
GLvoid glmClear(GLMmodel* model)
{
	GLMgroup* group;
	GLuint i;

	assert(model);

	if (model->pathname)     free(model->pathname);
	if (model->mtllibname) free(model->mtllibname);
	if (model->vertices)     free(model->vertices);
	if (model->normals)  free(model->normals);
	if (model->texcoords)  free(model->texcoords);
	if (model->colors)  free(model->colors);
	if (model->facetnorms) free(model->facetnorms);
	if (model->lines)
	{
		for (i=0; i<model->numlines; i++)
			free(L(i).vindices);
		free(model->lines);
	}
	if (model->triangles)  free(model->triangles);
	if (model->materials)
	{
		for (i = 0; i < model->nummaterials; i++)
		{
			free(model->materials[i].name);
			if (model->materials[i].havetexture &&
				glIsTexture(model->materials[i].textureID))
				glDeleteTextures(1, &model->materials[i].textureID);
		}
		free(model->materials);
	}
	while(model->groups)
	{
		group = model->groups;
		model->groups = model->groups->next;
		free(group->name);
		free(group->triangles);
		free(group);
	}

	model->pathname = 0;
	model->mtllibname = 0;
	model->numvertices = 0;
	model->vertices = 0;
	model->numnormals = 0;
	model->normals = 0;
	model->numtexcoords = 0;
	model->texcoords = 0;
	model->numcolors = 0;
	model->colors = 0;
	model->numfacetnorms = 0;
	model->facetnorms = 0;
	model->numlines = 0;
	model->numtriangles = 0;
	model->triangles = 0;
	model->nummaterials = 0;
	model->materials = 0;
	model->numgroups = 0;
	model->groups = 0;
	model->position[0] = 0.0f;
	model->position[1] = 0.0f;
	model->position[2] = 0.0f;
	model->hastexture = false;
}

void glmClearGeometry(GLMmodel* model)
{
	if (!model) return;

	free(model->vertices);      model->vertices = nullptr; model->numvertices = 0;
	free(model->normals);       model->normals = nullptr; model->numnormals = 0;
	free(model->texcoords);     model->texcoords = nullptr; model->numtexcoords = 0;
	free(model->colors);        model->colors = nullptr; model->numcolors = 0;
	free(model->facetnorms);    model->facetnorms = nullptr; model->numfacetnorms = 0;
	free(model->triangles);     model->triangles = nullptr; model->numtriangles = 0;
	free(model->lines);         model->lines = nullptr; model->numlines = 0;
	free(model->groups);        model->groups = nullptr; model->numgroups = 0;

	model->position[0] = 0.0f;
	model->position[1] = 0.0f;
	model->position[2] = 0.0f;
}

/* glmReadOBJ: Reads a model description from a Wavefront .OBJ file.
* Returns a pointer to the created object which should be free'd with
* glmDelete().
*
* filename - name of the file containing the Wavefront .OBJ format data.
*/
GLMmodel* glmReadOBJ(const char* filename, bool *no_fail)
{
	GLMmodel* model;
	FILE* file;

	/* open the file */
	std::string str = filename;
	if (!FOPEN(&file, str, "rb"))
		return 0;

	//find file size
	fseek(file, 0L, SEEK_END);
	long size = ftell(file);
	fseek(file, 0L, SEEK_SET);

	//read file to a string
	char* obj_content = (char*)malloc(size+1);
	if (!fread(obj_content, sizeof(char), size, file))
		return 0;
	obj_content[size] = 0;
	/* close the file */
	fclose(file);

	/* allocate a new model */
	model = (GLMmodel*)malloc(sizeof(GLMmodel));
	model->pathname    = STRDUP(filename);
	model->mtllibname    = NULL;
	model->numvertices   = 0;
	model->vertices    = NULL;
	model->numnormals    = 0;
	model->normals     = NULL;
	model->numtexcoords  = 0;
	model->texcoords       = NULL;
	model->numcolors = 0;
	model->colors = NULL;
	model->numfacetnorms = 0;
	model->facetnorms    = NULL;
	model->numtriangles  = 0;
	model->triangles       = NULL;
	model->numlines = 0;
	model->lines = NULL;
	model->nummaterials  = 0;
	model->materials       = NULL;
	model->numgroups       = 0;
	model->groups      = NULL;
	model->position[0]   = 0.0;
	model->position[1]   = 0.0;
	model->position[2]   = 0.0;
	model->hastexture = GL_FALSE;

	/* make a first pass through the file to get a count of the number
	of vertices, normals, texcoords & triangles */
	if (!glmFirstPass(model, obj_content))
		if(no_fail) *no_fail = false;

	/* allocate memory */
	if (model->numvertices)
		model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numvertices + 1));
	if (model->numlines)
		model->lines = (GLMline*)malloc(sizeof(GLMline) *
		model->numlines);
	if (model->numtriangles)
		model->triangles = (GLMtriangle*)malloc(sizeof(GLMtriangle) *
		model->numtriangles);
	if (model->numnormals)
		model->normals = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numnormals + 1));
	if (model->numtexcoords)
		model->texcoords = (GLfloat*)malloc(sizeof(GLfloat) *
		2 * (model->numtexcoords + 1));
	if (model->numcolors)
		model->colors = (GLfloat*)malloc(sizeof(GLfloat) *
		4 * (model->numcolors + 1));

	glmSecondPass(model, obj_content);

	free(obj_content);

	return model;
}

/* glmWriteOBJ: Writes a model description in Wavefront .OBJ format to
* a file.
*
* model - initialized GLMmodel structure
* filename - name of the file to write the Wavefront .OBJ format data to
* mode  - a bitwise or of values describing what is written to the file
*             GLM_NONE     -  render with only vertices
*             GLM_FLAT     -  render with facet normals
*             GLM_SMOOTH   -  render with vertex normals
*             GLM_TEXTURE  -  render with texture coords
*             GLM_COLOR    -  render with colors (color material)
*             GLM_MATERIAL -  render with materials
*             GLM_COLOR and GLM_MATERIAL should not both be specified.
*             GLM_FLAT and GLM_SMOOTH should not both be specified.
*/
GLvoid glmWriteOBJ(GLMmodel* model, const char* filename, GLuint mode)
{
	GLuint i;
	FILE* file;
	GLMgroup* group;

	assert(model);

	/* do a bit of warning */
	if (mode & GLM_FLAT && !model->facetnorms) {
		printf("glmWriteOBJ() warning: flat normal output requested "
			"with no facet normals defined.\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_SMOOTH && !model->normals) {
		printf("glmWriteOBJ() warning: smooth normal output requested "
			"with no normals defined.\n");
		mode &= ~GLM_SMOOTH;
	}
	if (mode & GLM_TEXTURE && !model->texcoords) {
		printf("glmWriteOBJ() warning: texture coordinate output requested "
			"with no texture coordinates defined.\n");
		mode &= ~GLM_TEXTURE;
	}
	if (mode & GLM_VERTC && !model->colors) {
		printf("glmWriteOBJ() warning: vertex color output requested "
			"with no vertex colors defined.\n");
		mode &= ~GLM_VERTC;
	}
	if (mode & GLM_FLAT && mode & GLM_SMOOTH) {
		printf("glmWriteOBJ() warning: flat normal output requested "
			"and smooth normal output requested (using smooth).\n");
		mode &= ~GLM_FLAT;
	}
	if (mode & GLM_COLOR && !model->materials) {
		printf("glmWriteOBJ() warning: color output requested "
			"with no colors (materials) defined.\n");
		mode &= ~GLM_COLOR;
	}
	if (mode & GLM_MATERIAL && !model->materials) {
		printf("glmWriteOBJ() warning: material output requested "
			"with no materials defined.\n");
		mode &= ~GLM_MATERIAL;
	}
	if (mode & GLM_COLOR && mode & GLM_MATERIAL) {
		printf("glmWriteOBJ() warning: color and material output requested "
			"outputting only materials.\n");
		mode &= ~GLM_COLOR;
	}


	/* open the file */
	std::string str = filename;
	if (!FOPEN(&file, str, "wt"))
		return;

	/* spit out a header */
	fprintf(file, "#  \n");
	fprintf(file, "#  Wavefront OBJ generated by GLM library\n");
	fprintf(file, "#  \n");
	fprintf(file, "#  GLM library\n");
	fprintf(file, "#  Nate Robins\n");
	fprintf(file, "#  ndr@pobox.com\n");
	fprintf(file, "#  http://www.pobox.com/~ndr\n");
	fprintf(file, "#  \n");

	if (mode & GLM_MATERIAL && model->mtllibname) {
		fprintf(file, "\nmtllib %s\n\n", model->mtllibname);
		glmWriteMTL(model, filename, model->mtllibname);
	}

	/* spit out the vertices */
	fprintf(file, "\n");
	fprintf(file, "# %d vertices\n", model->numvertices);
	for (i = 1; i <= model->numvertices; i++) {
		fprintf(file, "v %f %f %f\n",
			model->vertices[3 * i + 0],
			model->vertices[3 * i + 1],
			model->vertices[3 * i + 2]);
	}

	/* spit out the smooth/flat normals */
	if (mode & GLM_SMOOTH) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", model->numnormals);
		for (i = 1; i <= model->numnormals; i++) {
			fprintf(file, "vn %f %f %f\n",
				model->normals[3 * i + 0],
				model->normals[3 * i + 1],
				model->normals[3 * i + 2]);
		}
	} else if (mode & GLM_FLAT) {
		fprintf(file, "\n");
		fprintf(file, "# %d normals\n", model->numfacetnorms);
		for (i = 1; i <= model->numnormals; i++) {
			fprintf(file, "vn %f %f %f\n",
				model->facetnorms[3 * i + 0],
				model->facetnorms[3 * i + 1],
				model->facetnorms[3 * i + 2]);
		}
	}

	/* spit out the texture coordinates */
	if (mode & GLM_TEXTURE) {
		fprintf(file, "\n");
		fprintf(file, "# %d texcoords\n", model->numtexcoords);
		for (i = 1; i <= model->numtexcoords; i++) {
			fprintf(file, "vt %f %f\n",
				model->texcoords[2 * i + 0],
				model->texcoords[2 * i + 1]);
		}
	}

	/* spit out the colors */
	if (mode & GLM_VERTC) {
		fprintf(file, "\n");
		fprintf(file, "# %d colors\n", model->numcolors);
		for (i = 1; i <= model->numcolors; i++) {
			fprintf(file, "vc %f %f %f %f\n",
				model->colors[4 * i + 0],
				model->colors[4 * i + 1],
				model->colors[4 * i + 2],
				model->colors[4 * i + 3]);
		}
	}

	fprintf(file, "\n");
	fprintf(file, "# %d groups\n", model->numgroups);
	fprintf(file, "# %d faces (triangles)\n", model->numtriangles);
	fprintf(file, "\n");

	group = model->groups;
	while (group) {
		fprintf(file, "g %s\n", group->name);
		if (mode & GLM_MATERIAL)
			fprintf(file, "usemtl %s\n", model->materials[group->material].name);

		for (i = 0; i < group->numtriangles; i++) {
			if (mode & GLM_SMOOTH && mode & GLM_TEXTURE && mode & GLM_VERTC) {
				fprintf(file, "f %d/%d/%d/%d %d/%d/%d/%d %d/%d/%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).cindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).cindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).tindices[2],
					T(group->triangles[i]).nindices[2],
					T(group->triangles[i]).cindices[2]);
			}
			else if (mode & GLM_SMOOTH && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).tindices[2],
					T(group->triangles[i]).nindices[2]);
			}
			else if (mode & GLM_FLAT && mode & GLM_TEXTURE && mode & GLM_VERTC) {
				fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).cindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).cindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).cindices[2]);
			}
			else if (mode & GLM_FLAT && mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex);
			}
			else if (mode & GLM_TEXTURE && mode & GLM_VERTC) {
				fprintf(file, "f %d/%d/%d %d/%d/%d %d/%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).cindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).cindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).tindices[2],
					T(group->triangles[i]).cindices[2]);
			}
			else if (mode & GLM_TEXTURE) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).tindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).tindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).tindices[2]);
			}
			else if (mode & GLM_SMOOTH && mode & GLM_VERTC) {
				fprintf(file, "f %d//%d/%d %d//%d/%d %d//%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).cindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).cindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).nindices[2],
					T(group->triangles[i]).cindices[2]);
			}
			else if (mode & GLM_SMOOTH) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).nindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).nindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).nindices[2]);
			}
			else if (mode & GLM_FLAT && mode & GLM_VERTC) {
				fprintf(file, "f %d//%d/%d %d//%d/%d %d//%d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).cindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).cindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).cindices[2]);
			}
			else if (mode & GLM_FLAT) {
				fprintf(file, "f %d//%d %d//%d %d//%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).findex,
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).findex);
			}
			else if (mode & GLM_VERTC) {
				fprintf(file, "f %d/%d %d/%d %d/%d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).cindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).cindices[1],
					T(group->triangles[i]).vindices[2],
					T(group->triangles[i]).cindices[2]);
			}
			else {
				fprintf(file, "f %d %d %d\n",
					T(group->triangles[i]).vindices[0],
					T(group->triangles[i]).vindices[1],
					T(group->triangles[i]).vindices[2]);
			}
		}
		fprintf(file, "\n");
		group = group->next;
	}

	fclose(file);
}

/* glmWeld: eliminate (weld) vectors that are within an epsilon of
* each other.
*
* model   - initialized GLMmodel structure
* epsilon     - maximum difference between vertices
*               ( 0.00001 is a good start for a unitized model)
*
*/
GLvoid glmWeld(GLMmodel* model, GLfloat epsilon)
{
	GLfloat* vectors;
	GLfloat* copies;
	GLuint numvectors;
	GLuint i;

	/* vertices */
	numvectors = model->numvertices;
	vectors  = model->vertices;
	copies = glmWeldVectors(vectors, &numvectors, epsilon);

	int tri = 0;
	for (i = 0; i < model->numtriangles; i++)
	{
		GLuint ind0 = (GLuint)vectors[3 * T(i).vindices[0] + 0];
		GLuint ind1 = (GLuint)vectors[3 * T(i).vindices[1] + 0];
		GLuint ind2 = (GLuint)vectors[3 * T(i).vindices[2] + 0];

		if (ind0!=ind1 && ind0!=ind2 && ind0!=ind2)
		{
			T(tri).vindices[0] = ind0;
			T(tri).vindices[1] = ind1;
			T(tri).vindices[2] = ind2;
			tri++;
		}
	}
	model->numtriangles = tri;
	if (model->groups)
		model->groups->numtriangles = tri;

	/* free space for old vertices */
	free(vectors);

	/* allocate space for the new vertices */
	model->numvertices = numvectors;
	model->vertices = (GLfloat*)malloc(sizeof(GLfloat) *
		3 * (model->numvertices + 1));

	/* copy the optimized vertices into the actual vertex list */
	for (i = 1; i <= model->numvertices; i++) {
		model->vertices[3 * i + 0] = copies[3 * i + 0];
		model->vertices[3 * i + 1] = copies[3 * i + 1];
		model->vertices[3 * i + 2] = copies[3 * i + 2];
	}

	free(copies);
}

/* glmReadPPM: read a PPM raw (type P6) file.  The PPM file has a header
* that should look something like:
*
*    P6
*    # comment
*    width height max_value
*    rgbrgbrgb...
*
* where "P6" is the magic cookie which identifies the file type and
* should be the only characters on the first line followed by a
* carriage return.  Any line starting with a # mark will be treated
* as a comment and discarded.   After the magic cookie, three integer
* values are expected: width, height of the image and the maximum
* value for a pixel (max_value must be < 256 for PPM raw files).  The
* data section consists of width*height rgb triplets (one byte each)
* in binary format (i.e., such as that written with fwrite() or
* equivalent).
*
* The rgb data is returned as an array of unsigned chars (packed
* rgb).  The malloc()'d memory should be free()'d by the caller.  If
* an error occurs, an error message is sent to stderr and NULL is
* returned.
*
* filename   - name of the .ppm file.
* width      - will contain the width of the image on return.
* height     - will contain the height of the image on return.
*
*/
GLubyte* glmReadPPM(char* filename, int* width, int* height)
{
	FILE* fp;
	int i, w, h, d;
	unsigned char* image;
	char head[70];          /* max line <= 70 in PPM (per spec). */

	std::string str = filename;
	if (!FOPEN(&fp, str, "rb"))
		return NULL;

	/* grab first two chars of the file and make sure that it has the
	correct magic cookie for a raw PPM file. */
	fgets(head, 70, fp);
	if (strncmp(head, "P6", 2)) {
		fprintf(stderr, "%s: Not a raw PPM file\n", filename);
		return NULL;
	}

	/* grab the three elements in the header (width, height, maxval). */
	i = 0;
	while(i < 3) {
		fgets(head, 70, fp);
		if (head[0] == '#')     /* skip comments. */
			continue;
		if (i == 0)
			i += SSCANF(head, "%d %d %d", &w, &h, &d);
		else if (i == 1)
			i += SSCANF(head, "%d %d", &h, &d);
		else if (i == 2)
			i += SSCANF(head, "%d", &d);
	}

	/* grab all the image data in one fell swoop. */
	image = (unsigned char*)malloc(sizeof(unsigned char)*w*h*3);
	fread(image, sizeof(unsigned char), w*h*3, fp);
	fclose(fp);

	*width = w;
	*height = h;
	return image;
}
