#include "tests.h"
#include "asserts.h"
#include <Table.h>
#include <RecordHistParams.h>
#include <Global.h>

void TableTest()
{
	flrd::Table table;

	flrd::EntryHist* eh = new flrd::EntryHist();
	flrd::Params* params = glbin.get_params("comp_gen");
	flrd::EntryParams* ep = new flrd::EntryParams();
	ep->setParams(params);
	flrd::RecordHistParams* rec = new flrd::RecordHistParams();

	eh->setRange(0.0f, 255.0f);
	eh->setPopulation(1e6f);
	unsigned int hist[256];
	for (int i = 0; i < 256; ++i)
		hist[i] = rand() % 100;
	eh->setData(hist);

	ep->setParam("iter", 50);
	ep->setParam("thresh", 0.5);
	ep->setParam("use_dist_field", true);
	ep->setParam("dist_strength", 0.52);
	ep->setParam("dist_filter_size", 3);
	ep->setParam("max_dist", 30);
	ep->setParam("dist_thresh", 0.25);
	ep->setParam("diff", true);
	ep->setParam("falloff", 0.01);
	ep->setParam("density", true);
	ep->setParam("density_thresh", 1);
	ep->setParam("varth", 0.0001);
	ep->setParam("density_window_size", 5);
	ep->setParam("density_stats_size", 15);
	ep->setParam("cleanb", true);
	ep->setParam("clean_iter", 5);
	ep->setParam("clean_size_vl", 5);
	ep->setParam("grow_fixed", true);

	rec->setInput(eh);
	rec->setOutput(ep);

	table.addRecord(rec);

	//do something
	ep->setParam("invalid", 123);
}