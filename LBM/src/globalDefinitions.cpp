/**
 * Description: Implementation of the global definitions.
 * Created: 8 Nov. 2016
 * Author: Roland Guichard
 */

#include "globalDefinitions.h"

lattice::lattice(int LX, int LY, int LZ):

	/*
	 * Lattice constructor. Note there is no initialisation list.
	 */

	lx(LX), ly(LY), lz(LZ) {
	Q0 = new float[lz*ly*lx];
	Q1 = new float[lz*ly*lx];
	Q2 = new float[lz*ly*lx];
	Q3 = new float[lz*ly*lx];
	Q4 = new float[lz*ly*lx];
	Q5 = new float[lz*ly*lx];
	Q6 = new float[lz*ly*lx];
	Q7 = new float[lz*ly*lx];
	Q8 = new float[lz*ly*lx];
	Q9 = new float[lz*ly*lx];
	Q10 = new float[lz*ly*lx];
	Q11 = new float[lz*ly*lx];
	Q12 = new float[lz*ly*lx];
	Q13 = new float[lz*ly*lx];
	Q14 = new float[lz*ly*lx];
	Q15 = new float[lz*ly*lx];
	Q16 = new float[lz*ly*lx];
	Q17 = new float[lz*ly*lx];
	Q18 = new float[lz*ly*lx];
}

lattice::lattice(int LX, int LY, int LZ, int dump):

	/*
	 * Lattice constructor overloaded
	 */

	lx(LX), ly(LY), lz(LZ){
	Q0 = new float[lz*ly*lx];
	Q1 = new float[lz*ly*lx];
	Q2 = new float[lz*ly*lx];
	Q3 = new float[lz*ly*lx];
	Q4 = new float[lz*ly*lx];
	Q5 = new float[lz*ly*lx];
	Q6 = new float[lz*ly*lx];
	Q7 = new float[lz*ly*lx];
	Q8 = new float[lz*ly*lx];
	Q9 = new float[lz*ly*lx];
	Q10 = new float[lz*ly*lx];
	Q11 = new float[lz*ly*lx];
	Q12 = new float[lz*ly*lx];
	Q13 = new float[lz*ly*lx];
	Q14 = new float[lz*ly*lx];
	Q15 = new float[lz*ly*lx];
	Q16 = new float[lz*ly*lx];
	Q17 = new float[lz*ly*lx];
	Q18 = new float[lz*ly*lx];

	//	int FLOATING_array_size=lx*ly*lz*sizeof(FLOATING);
	//		cudaMalloc((void **)&Q0, FLOATING_array_size);
	//		cudaMalloc((void **)&Q1, FLOATING_array_size);
	//		cudaMalloc((void **)&Q2, FLOATING_array_size);
	//		cudaMalloc((void **)&Q3, FLOATING_array_size);
	//		cudaMalloc((void **)&Q4, FLOATING_array_size);
	//		cudaMalloc((void **)&Q5, FLOATING_array_size);
	//		cudaMalloc((void **)&Q6, FLOATING_array_size);
	//		cudaMalloc((void **)&Q7, FLOATING_array_size);
	//		cudaMalloc((void **)&Q8, FLOATING_array_size);
	//		cudaMalloc((void **)&Q9, FLOATING_array_size);
	//		cudaMalloc((void **)&Q10, FLOATING_array_size);
	//		cudaMalloc((void **)&Q11, FLOATING_array_size);
	//		cudaMalloc((void **)&Q12, FLOATING_array_size);
	//		cudaMalloc((void **)&Q13, FLOATING_array_size);
	//		cudaMalloc((void **)&Q14, FLOATING_array_size);
	//		cudaMalloc((void **)&Q15, FLOATING_array_size);
	//		cudaMalloc((void **)&Q16, FLOATING_array_size);
	//		cudaMalloc((void **)&Q17, FLOATING_array_size);
	//		cudaMalloc((void **)&Q18, FLOATING_array_size);
}

lattice::~lattice() {

	/*
	 * Lattice destructor
	 */

	delete [] Q0;
	delete [] Q1;
	delete [] Q2;
	delete [] Q3;
	delete [] Q4;
	delete [] Q5;
	delete [] Q6;
	delete [] Q7;
	delete [] Q8;
	delete [] Q9;
	delete [] Q10;
	delete [] Q11;
	delete [] Q12;
	delete [] Q13;
	delete [] Q14;
	delete [] Q15;
	delete [] Q16;
	delete [] Q17;
	delete [] Q18;

	printf("host memories deleted!\n");
}

LBM::LBM(const int& LX, const int& LY, const int& LZ, const float& DENSITY,
		 const float& T0, const float& T1, const float& T2, const float& CSQR):

	/**
	 * LBM constructor
	 */

	lx(LX), ly(LY), lz(LZ),
	latticeNodes(lx*ly*lz),
	noObstacleLatticesAtPenultimateXSlice(0),
	threadsForStreamingCollisionAndRelaxation(512),
	blocksForStreamingCollisionAndRelaxation(32),
	sizeOfAllocatedSharedMemoryForStreamingCollisionAndRelaxation(48*1024),
	convectiveBoundaryConditionsBlocks(32),
	maxIterations(1000), checkStep(100),
	timeElapsed(0.),
	/* Private variables */
	timeUnit(0),
	twoDimensionalLength(ly*lz),
	threeDimensionalLength(lx*ly*lz),
	floatingSliceSize(twoDimensionalLength*sizeof(float)),
	intArraySize(threeDimensionalLength*sizeof(int)),
	nu(0.0175), rSmall(6.67897), reynoldsNb(195.732), s(23.7849), density(DENSITY),
	prDiff(0.0), prOut(0.0), prIn(0.0), vor(0.0),
	t0(density*T0), t1(density*T1), t2(density*T2), cSqr(), reciprocalCSqr(1.0/cSqr),
	tau(3.0*nu + 0.5), omega(1.0/tau), oneMinusOmega (1.0-omega),
	D3(lx, ly, lz), D3Help(lx, ly, lz),
	D3_d(lx, ly, lz, 0), D3Help_d(lx, ly, lz, 0),
	/* Pointers */
	obstacles(new int[lz*ly*lx]), obstacles_d(NULL),
	uCurrent(new float[ly*lz]),  vCurrent(new float[ly*lz]),  wCurrent(new float[ly*lz]),
	uPreviousSpatialBoundary(new float[ly*lz]), vPreviousSpatialBoundary(new float[ly*lz]), wPreviousSpatialBoundary(new float[ly*lz]),
	uPreviousTemporalBoundary(new float[ly*lz]), vPreviousTemporalBoundary(new float[ly*lz]), wPreviousTemporalBoundary(new float[ly*lz]),
	uCurrent_d(NULL), uCurrentTemp_d(NULL), vCurrent_d(NULL), wCurrent_d(NULL),
	uPreviousSpatialBoundary_d(NULL), vPreviousSpatialBoundary_d(NULL), wPreviousSpatialBoundary_d(NULL),
	uPreviousTemporalBoundary_d(NULL),vPreviousTemporalBoundary_d(NULL), wPreviousTemporalBoundary_d(NULL),
	tempCPU_uCurrent_d(NULL), tempCPU_vCurrent_d(NULL), tempCPU_wCurrent_d(NULL),
	tempCPU_uPreviousTemporalBoundary_d(NULL), tempCPU_vPreviousTemporalBoundary_d(NULL), tempCPU_wPreviousTemporalBoundary_d(NULL),
	tempCPU_uPreviousSpatialBoundary_d(NULL), tempCPU_vPreviousSpatialBoundary_d(NULL), tempCPU_wPreviousSpatialBoundary_d(NULL),
	tempCheckDensity_d(NULL), tempCheckDensity_d_full(NULL),
	Ux(new float[lx*ly*lz]),
	Uy(new float[lx*ly*lz]),
	Uz(new float[lx*ly*lz]),
	Pressure(new float[lx*ly*lz]),
	Wx(new float[lx*ly*lz]),
	Wy(new float[lx*ly*lz]),
	Wz(new float[lx*ly*lz]) {

	cout << "***LBM Starting***" << endl;

	time (&timeStart);

	readExternalConfigurationFileForTheSolver("./input/LBMConfiguration.inp");

	resetConvergenceFile();

//	calculate_CUDA_quantities();
//	display_CUDA_specifications();

	initialiseAllDataArrays();
	displayLBMSpecifications();
//	abstract_initialise();
}

void LBM::initialiseAllDataArrays() {

	/* Calls all the initialisations and allocations */

	initialiseHostData();
	allocateDeviceArrays();
	initialiseDeviceData();
}

void LBM::initialiseHostData() {

}

void LBM::allocateDeviceArrays() {

}

void LBM::initialiseDeviceData() {

}

void LBM::resetConvergenceFile() {

	/* Delete and create a new LBMConvergence.dat file */

	if ( remove("./output/LBMConvergence.dat") != 0 ) cout << "Couldn't delete LBMConvergence.dat" << endl;
	else cout << "Create LBMConvergence.dat" << endl;

	ofstream convergenceFile("./output/LBMConvergence.dat");

	convergenceFile << "# Iteration \t Convergence value" << endl;
	convergenceFile.close();

}

void LBM::readExternalConfigurationFileForTheSolver(const string filename) {

	/*
	 * Reads the input values from the LBMGeometry input file.
	 */

	vector<string> configurationParameters;
	ifstream configurationFile(filename.c_str());
	string buffer;
	if(configurationFile.is_open()){
		while (configurationFile >> buffer) {
			configurationParameters.push_back(buffer);
		}
		cout << "Configuration parameters read from: " << filename.c_str() << endl;
		maxIterations = atoi(configurationParameters[0].c_str());
		cout << "\t Max iterations: " << maxIterations << endl;

		/* Check step: perform density check and export */
		checkStep = atoi(configurationParameters[1].c_str());
		cout << "\t Check step: " << checkStep << endl;

		nu = atof(configurationParameters[2].c_str());
		cout << "\t nu: " << nu << endl;

		rSmall = atof(configurationParameters[3].c_str());
		cout << "\t rSmall: " << rSmall << endl;

		reynoldsNb = atof(configurationParameters[4].c_str());
		cout << "\t reynoldsNb: " << reynoldsNb << endl;

		s = atof(configurationParameters[5].c_str());
		cout << "\t s: " << s << endl;

		baffle = atoi(configurationParameters[6].c_str());
		cout << "\t baffle position on X=" << baffle << endl;

		threadsPerKernel = atoi(configurationParameters[7].c_str());
		cout << "\t CUDA threads per kernel: " << threadsPerKernel << endl;

		caseName = configurationParameters[8].c_str();
		cout << "Case: " << caseName << endl;

		cout << "In total: " << configurationParameters.size() << " parameters were read" << endl;
		configurationFile.close();
	}
}

void LBM::createAnExampleConfigurationFile(const string exampleFileName) {

	/*
     * Reads the input values from the LBMGeometry input file.
     */

	ofstream exampleFile( exampleFileName );
	if (exampleFile.is_open()){
		exampleFile << "10" << endl;
		exampleFile << "100" << endl;
		exampleFile << "0.0175" << endl;
		exampleFile << "7" << endl;
		exampleFile << "100" << endl;
		exampleFile << "26" << endl;
		exampleFile << "59" << endl;
		exampleFile << "512" << endl;
		exampleFile << "datum_design_case_name1" << endl;
		exampleFile.close();
	}
}

void readLBMGeometriesFromFile (int &lx, int &ly, int &lz, int &nbDensities, const string filePathAndName) {

	/*
	 * Reads the input values from the LBMGeometry input file.
	 */

	vector<string> geometryParameters;
	ifstream configFileReader( filePathAndName.c_str() ); // Why c_str ?
//	ifstream configFileReader( filePathAndName );
	string buffer;

	// Read lines from the file
	if (configFileReader.is_open()){
		while (configFileReader >> buffer) {
			geometryParameters.push_back(buffer);
		}
		cout << "Geometry parameters read from: " << filePathAndName.c_str() << endl;
		lx = atoi(geometryParameters[0].c_str());
		cout << "\t Domain length in x: " << lx << endl;

		ly = atoi(geometryParameters[1].c_str());
		cout << "\t Domain length in y: " << ly << endl;

		lz = atoi(geometryParameters[2].c_str());
		cout << "\t Domain length in z: " << lz << endl;

		nbDensities = atoi(geometryParameters[3].c_str());
		cout << "\t Number of densities on each node: " << nbDensities << endl;

		cout <<"In total: " << geometryParameters.size() << " parameters were read." << endl;
		configFileReader.close();
	}
	else {
		cout << "The file " << filePathAndName << " was not found" << endl;
		cout << "Create a new file in the ./input directory with 3 lines (one number on every line), each corresponding to the respective dimension of x, y and z." << endl;
		// Exit the whole program with an error.
		exit (-1);
	}
//	cout << filePathAndName.c_str() <<  endl;
//	cout << 2;

}
