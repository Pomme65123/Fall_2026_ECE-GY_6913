#include <iostream>
#include <fstream>
#include <math.h>
#include <vector>
#include <bitset>

using namespace std;

int main(int argc, char **argv)
{
	ifstream config;
	config.open(argv[1]);

	int m, w, h;
	config >> m;
	config >> h;
	config >> w;

	config.close();

	ofstream out;
	string out_file_name = string(argv[2]) + ".out";
	out.open(out_file_name.c_str());

	ifstream trace;
	trace.open(argv[2]);

	std::vector<unsigned int> bht(1 << h, 0);
	std::vector<unsigned int> pht(1 << m, 2);

	unsigned long pc{};

	int taken{};

	while (trace >> std::hex >> pc >> std::dec >> taken)
	{

		unsigned long shifted_pc = pc >> 2;
		unsigned int bht_mask = (1 << h) - 1;
		unsigned int bht_index = shifted_pc & bht_mask;

		unsigned int pc_part_mask = (1 << (m - w)) - 1;
		unsigned int pc_part = shifted_pc & pc_part_mask;
		unsigned int pht_index = (pc_part << w) | bht[bht_index];

		int prediction = (pht[pht_index] >= 2) ? 1 : 0;
		out << prediction << endl;

		if (taken == 1)
		{
			if (pht[pht_index] < 3)
				pht[pht_index]++;
		}
		else
		{
			if (pht[pht_index] > 0)
				pht[pht_index]--;
		}

		unsigned int history_mask = (1 << w) - 1;
		bht[bht_index] = ((bht[bht_index] << 1) | taken) & history_mask;
	}

	trace.close();
	out.close();
}

// Path: branchsimulator_skeleton_23.cpp