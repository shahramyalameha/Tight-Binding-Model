/*
 * TightBinding.cpp

 *
 *  Created on: Apr 13, 2017
 *      Author: johnstanco
 */

#include "../include/tightBinding.hpp"

/*
 *   This program is designed to read in data from a text file produced by Wannier90, containing the following columns:
 *
 *  The program will organize the data into a hermitian matrix, computing eigenvalues for a range of k-values
 *  over specific high-symmetry points.
 *
 *  The program will plot the energy eigenvalues vs k over these specified paths, resulting in a band structure
 *  plot.
 */

using namespace arma;

int
main(int argc, char* argv[])
{
  if(argc != 2)
  {
    cerr << "routine tbBands: Improper number of command line arguments specified (1)" << std::endl;
  }
  else
  {
    clock_t t = clock();
    std::string seedname = argv[1];
    TightBinding tb(seedname);
    try
    {
      tb.computeBands();
      std::cout << "\nFinished" << std::endl;
      t = clock() - t;
      int  time = t / CLOCKS_PER_SEC,
      hrs = time / 3600,
      min = time / 60 - hrs * 60,
      sec = time - (hrs * 3600 + min * 60);
      std::cout << "Time Elapsed:   " <<  hrs << " hours, " << min << " minutes, " << sec << " seconds" << std::endl;
    }
    catch(std::string err)
    {
      cerr << err << std::endl;
    }
    catch(...)
    {
      return EXIT_FAILURE;
    }
  }
  return 0;
}
