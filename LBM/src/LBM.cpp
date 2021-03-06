/**
 * @file LBM.cpp
 * @brief Implementation of the LBM class.
 * @date 8 Nov. 2016
 * @author Roland Guichard
 */

#include "gtest.h"
#include "globalDefinitions.h"

LBM::LBM(const int& LX, const int& LY, const int& LZ, const int& MAXITERATIONS,
		const int& CHECKSTEP, const float& NU, const float& RSMALL,
		const float& REYNOLDSNB, const float& S, const int& BAFFLE,
		const int& THREADSPERKERNEL, const string& CASENAME,
		const float& DENSITY, const float& T0, const float& T1,
		const float& T2, const float& CSQR):
	/**
	 * @brief LBM constructor. Initialise all necessary class members
	 * @param LX (const int) domain dimension in x-coordinate
	 * @param LY (const int) domain dimension in y-coordinate
	 * @param LZ (const int) domain dimension in z-coordinate
	 * @param MAXITERATIONS (const int) Maximum iterations
	 * @param CHECKSTEP (const int) check step
	 * @param NU (const float)
	 * @param RSMALL (const float)
	 * @param REYNOLDSNB (const float) Reynolds Number
	 * @param S (const float)
	 * @param BAFFLE (const int)
	 * @param THREADSPERKERNEL (const int)
	 * @param CASENAME (const string)
	 * @param DENSITY (const flt)
	 * @param T0 (const flt)
	 * @param T1 (const flt)
	 * @param T2 (const flt)
	 * @param CSQR (const flt)
	 * @return none
	 */
//	Public variables
	lx { LX },
	ly { LY },
	lz { LZ },
	latticeNodes { lx * ly * lz },
	noObstacleLatticesAtPenultimateXSlice { 0 },
	threadsForStreamingCollisionAndRelaxation { 512 },
	blocksForStreamingCollisionAndRelaxation { 32 },
	sizeOfAllocatedSharedMemoryForStreamingCollisionAndRelaxation { 48 * 1024 },
	convectiveBoundaryConditionsBlocks { 32 },
	maxIterations { MAXITERATIONS },
	checkStep { CHECKSTEP },
	baffle { BAFFLE },
	threadsPerKernel { THREADSPERKERNEL },
	timeElapsed { 0. },
	nu { NU },
	rSmall { RSMALL },
	reynoldsNb { REYNOLDSNB },
	s { S },
	caseName { CASENAME },

//	Private variables
	timeUnit { 0 },
	twoDimensionalLength { ly * lz },
	threeDimensionalLength { lx * ly * lz },
	floatingSliceSize(twoDimensionalLength*sizeof(float)),
	intArraySize(threeDimensionalLength*sizeof(int)),
	density(DENSITY),
	prDiff(0.0),
	prOut(0.0),
	prIn(0.0),
	vor(0.0),
	t0 { density * T0 },
	t1 { density * T1 },
	t2 { density * T2 },
	cSqr(CSQR),
	reciprocalCSqr(1.0 / cSqr),
	tau(3.0*nu + 0.5),
	omega(1.0/tau),
	oneMinusOmega (1.0-omega),
	/* Lattice instantiations */
	D3(lx, ly, lz),
	D3Help(lx, ly, lz),
	D3_d(lx, ly, lz, 0),
	D3Help_d(lx, ly, lz, 0),
	/* Pointers */
	obstacles(new int[lz * ly * lx] { }),
	obstacles_d(NULL),
	uCurrent( new float[ly * lz] { }),
	vCurrent( new float[ly * lz] { }),
	wCurrent( new float[ly * lz] { }),
	uPreviousSpatialBoundary( new float[ly * lz] { }),
	vPreviousSpatialBoundary( new float[ly * lz] { }),
	wPreviousSpatialBoundary( new float[ly * lz] { }),
	uPreviousTemporalBoundary( new float[ly * lz] { }),
	vPreviousTemporalBoundary( new float[ly * lz] { }),
	wPreviousTemporalBoundary( new float[ly * lz] { }),
	uCurrent_d(NULL),
	uCurrentTemp_d(NULL),
	vCurrent_d(NULL),
	wCurrent_d(NULL),
	uPreviousSpatialBoundary_d(NULL),
	vPreviousSpatialBoundary_d(NULL),
	wPreviousSpatialBoundary_d(NULL),
	uPreviousTemporalBoundary_d(NULL),
	vPreviousTemporalBoundary_d(NULL),
	wPreviousTemporalBoundary_d(NULL),
	tempCPU_uCurrent_d(NULL),
	tempCPU_vCurrent_d(NULL),
	tempCPU_wCurrent_d(NULL),
	tempCPU_uPreviousTemporalBoundary_d(NULL),
	tempCPU_vPreviousTemporalBoundary_d(NULL),
	tempCPU_wPreviousTemporalBoundary_d(NULL),
	tempCPU_uPreviousSpatialBoundary_d(NULL),
	tempCPU_vPreviousSpatialBoundary_d(NULL),
	tempCPU_wPreviousSpatialBoundary_d(NULL),
	tempCheckDensity_d(NULL),
	tempCheckDensity_d_full(NULL),
	Ux( new float[lx * ly * lz] { }),
	Uy(new float[lx * ly * lz] { }),
	Uz( new float[lx * ly * lz] { }),
	Pressure( new float[lx * ly * lz] { }),
	Wx( new float[lx * ly * lz] { }),
	Wy( new float[lx * ly * lz] { }),
	Wz( new float[lx * ly * lz] { }) {

	cout << "***LBM Starting***" << endl;

	time (&timeStart);

	resetConvergenceFile();

//	calculate_CUDA_quantities();
//	display_CUDA_specifications();

	testInitialisationOfPrivateMembers();

	initialiseAllDataArrays();
	displayLBMSpecifications();
	abstractInitialise();
}

LBM::~LBM() {

}

void LBM::testInitialisationOfPrivateMembers() {

	EXPECT_EQ(0, timeUnit);
	EXPECT_EQ(ly * lz, twoDimensionalLength);
	EXPECT_EQ(lx * ly * lz, threeDimensionalLength);
	EXPECT_EQ(twoDimensionalLength*sizeof(float), floatingSliceSize);
	EXPECT_EQ(threeDimensionalLength*sizeof(int), intArraySize);
	EXPECT_EQ(density, density);
	EXPECT_EQ(0.0, prDiff);
	EXPECT_EQ(0.0, prOut);
	EXPECT_EQ(0.0, prIn);
	EXPECT_EQ(0.0, vor);
	EXPECT_EQ(density * t0, t0);
	EXPECT_EQ(density * t1, t1);
	EXPECT_EQ(density * t2, t2);

	EXPECT_EQ(cSqr, cSqr);
	EXPECT_FLOAT_EQ(1./cSqr, reciprocalCSqr);
	EXPECT_FLOAT_EQ(3.0*nu + 0.5, tau);
	EXPECT_FLOAT_EQ(1./(3.0*nu + 0.5), omega);
	EXPECT_FLOAT_EQ(1.-1./(3.0*nu + 0.5), oneMinusOmega);
}

void LBM::abstractInitialise() {

	/**
	 * @brief Calls all the initialisations and allocations methods in the host
	 * @return none
	 */

	float tempDensity = 0.0;

	initialiseMicroscopicDensityArraysInTheHost();
	calculateMacroscopicDensityInTheHost(tempDensity);
	createReactorGeometryInTheHost();

}

void LBM::initialiseMicroscopicDensityArraysInTheHost() {

	/* *
	 * @brief Initialise directional density distribution functions f_n with equilibrium for zero velocity,
	 * compute the weighting factors (depending on lattice geometry) and loop over the computational domain.
	 * @return none
	 */

	int x { }, y { }, z { };

	for (z = 0; z < lz ; ++z) {
		for (y = 0; y < ly; ++y) {
			for (x = 0; x < lx; ++x) {

				/* Zero velocity density */
				D3.Q0[index(z,y,x)]     = t0;
				D3Help.Q0[index(z,y,x)] = t0;

				/* *
				 * Equilibrium densities for velocities in the Ox, Oy, Oz axes in the D3Q19 nomenclatura
				 * pointing to the centres of the lattice faces.
				 */

				D3.Q1[index(z,y,x)]     = t1;
				D3.Q2[index(z,y,x)]     = t1;
				D3.Q3[index(z,y,x)]     = t1;
				D3.Q4[index(z,y,x)]     = t1;
				D3.Q5[index(z,y,x)]     = t1;
				D3.Q6[index(z,y,x)]     = t1;

				D3Help.Q1[index(z,y,x)] = t1;
				D3Help.Q2[index(z,y,x)] = t1;
				D3Help.Q3[index(z,y,x)] = t1;
				D3Help.Q4[index(z,y,x)] = t1;
				D3Help.Q5[index(z,y,x)] = t1;
				D3Help.Q6[index(z,y,x)] = t1;

				/* *
				 * Equilibrium densities for velocities in the diagonal axes in the D3Q19 nomenclatura
				 * pointing to the centres of the lattice edges.
				 */

				D3.Q7[index(z,y,x)]     = t2;
				D3.Q8[index(z,y,x)]     = t2;
				D3.Q9[index(z,y,x)]     = t2;
				D3.Q10[index(z,y,x)]    = t2;
				D3.Q11[index(z,y,x)]    = t2;
				D3.Q12[index(z,y,x)]    = t2;
				D3.Q13[index(z,y,x)]    = t2;
				D3.Q14[index(z,y,x)]    = t2;
				D3.Q15[index(z,y,x)]    = t2;
				D3.Q16[index(z,y,x)]    = t2;
				D3.Q17[index(z,y,x)]    = t2;
				D3.Q18[index(z,y,x)]    = t2;

				D3Help.Q7[index(z,y,x)] = t2;
				D3Help.Q8[index(z,y,x)] = t2;
				D3Help.Q9[index(z,y,x)] = t2;
				D3Help.Q10[index(z,y,x)]= t2;
				D3Help.Q11[index(z,y,x)]= t2;
				D3Help.Q12[index(z,y,x)]= t2;
				D3Help.Q13[index(z,y,x)]= t2;
				D3Help.Q14[index(z,y,x)]= t2;
				D3Help.Q15[index(z,y,x)]= t2;
				D3Help.Q16[index(z,y,x)]= t2;
				D3Help.Q17[index(z,y,x)]= t2;
				D3Help.Q18[index(z,y,x)]= t2;

			}
		}
	}
}

int LBM::calculateMacroscopicDensityInTheHost(float& density) {

	/**
	 * @brief Compute the integral density
	 * @param density (flt)
	 * @return successful exit
	 */

	//if(dataLocation == GPU)
	//copyDataFromDeviceToHost

	int x, y, z;
	float nSum = 0.;

	/**
	 * Loops over the last quarter of the computational domain and all densities
	 */

	for (z = 0; z < lz; ++z) {
		for (y = 0; y < ly; ++y) {
			for (x = (lx*3/4)-1; x < lx; ++x) {
				nSum+= sumMicroscopicDensitiesForSingleNode(x,y,z);
			}
		}
	}

	cout.setf(ios::fixed, ios::floatfield);
	cout.precision(10);
	cout << "Density check: Integral density=" << nSum << " " << nSum/(lz*ly*(lx/4)) << endl;

	return 0;
}

float LBM::sumMicroscopicDensitiesForSingleNode(const int x, const int y, const int z) {

	/**
	 * @brief Compute the integral density at a single lattice node
	 * @param x (int) node x-coordinate
	 * @param y (int) node y-coordinate
	 * @param z (int) node z-coordinate
	 * @return sum (flt) integral density
	 */

	float sum = 0.;

	sum+= D3.Q0[index(z,y,x)];
	sum+= D3.Q1[index(z,y,x)];
	sum+= D3.Q2[index(z,y,x)];
	sum+= D3.Q3[index(z,y,x)];
	sum+= D3.Q4[index(z,y,x)];
	sum+= D3.Q5[index(z,y,x)];
	sum+= D3.Q6[index(z,y,x)];
	sum+= D3.Q7[index(z,y,x)];
	sum+= D3.Q8[index(z,y,x)];
	sum+= D3.Q9[index(z,y,x)];
	sum+= D3.Q10[index(z,y,x)];
	sum+= D3.Q11[index(z,y,x)];
	sum+= D3.Q12[index(z,y,x)];
	sum+= D3.Q13[index(z,y,x)];
	sum+= D3.Q14[index(z,y,x)];
	sum+= D3.Q15[index(z,y,x)];
	sum+= D3.Q16[index(z,y,x)];
	sum+= D3.Q17[index(z,y,x)];
	sum+= D3.Q18[index(z,y,x)];

	return sum;
}

void LBM::createReactorGeometryInTheHost() {

}


void LBM::displayLBMSpecifications() {
	cout << "Displaying LBM (built-in quantities)" << endl;
	cout << "\t density: " << density << endl;
	cout << "\t t0: " << t0 << endl;
	cout << "\t t1: " << t1 << endl;
	cout << "\t t2: " << t2 << endl;
	cout << "\t cSqr: " << cSqr << endl;
	cout << "\t tau: " << tau << endl;
	cout << "\t omega: " << omega << endl;
}

void LBM::initialiseAllDataArrays() {

	/**
	 * @brief Calls all the initialisations and allocations methods
	 */

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

	/**
	 * @brief Delete and create a new LBMConvergence.dat file if none
	 */

	if ( remove("./output/LBMConvergence.dat") != 0 ) cout << "Couldn't delete LBMConvergence.dat" << endl;
	else cout << "Create LBMConvergence.dat" << endl;

	ofstream convergenceFile("./output/LBMConvergence.dat");

	convergenceFile << "# Iteration \t Convergence value" << endl;
	convergenceFile.close();

}


void LBM::createAnExampleConfigurationFile(const string exampleFileName) {

	/**
	 * @brief Creates a dummy output file.
	 * @param exampleFileName (const str) file name
	 * @return none
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
