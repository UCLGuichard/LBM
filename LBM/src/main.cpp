/**
 * @file main.cpp
 * @author Dr. Roland Guichard <r.guichard@ucl.ac.uk>
 * @version 1.0
 * @date 8 Nov. 2016
 * @copyright L-GPL
 * @brief CUDA/OpenACC Lattice-Boltzmann in C++ as a in-depth project.
 * Adapted from Christos Tsotskas's LBM code(itself adapted from a Fortran code by Prof. Djenidi)
 * LBM_configuration.txt, LBM_geometry.txt
 *               Output: geometry, convergence, report, data.
 *               Output files start with "LBM_"
 */

// Possible CUDA extensions
#include "globalDefinitions.h"

//using namespace std;

int main() {

	/**
	 * @brief Main for the LBM method.
	 * Read input files for the domain dimensions and instantiate the LBM method.
	 */

	/* Declare and initialise domain values */
	int lx = 1000, ly = 1000, lz = 1000, nbDensities = 100;

	/* Input file */
	string filePathAndName = "./input/LBMGeometry.inp";

	/* Read the file */
	readLBMGeometriesFromFile (lx, ly, lz, nbDensities, filePathAndName);

	/* Instantiate and run the LBM */
	LBM myLBM (lx, ly, lz, 1.0, 1.0/3.0, 1.0/18.0, 1.0/36.0, 1.0/3.0);
//	myLBM.createAnExampleConfigurationFile("exampleFielName.txt");
//	LBM myLBM( lx, ly, lz, 1.0,  1.0/3.0 , 1.0/18.0, 1.0/36.0, 1.0/3.0);
//	cout << fileName;
	return 0;
}
