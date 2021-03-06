//
//  TightBinding.cpp
//  
//
//  Created by John Stanco on 6/28/17.
//
//

#include "../include/tightBinding.hpp"

/******* Constructors *******/

#define hBar 6.58211951440e-16
std::complex<double> I = std::complex<double>(0, 1);
double pi = std::acos(-1);

TightBinding::TightBinding(){}

TightBinding::TightBinding(std::string seed)
{
  seedname = seed;
  lat = readWinFile(seedname);
  std::vector<mat> dat = read_hrFile(seedname);
  hr_dat = dat[0];
  //hr_dat.print();
  hr_weights = dat[1];
  wsvec_dat = dat[2];
  wsvec_weights = dat[3];
  int curr = hr_dat(0, lat.dim()), prev = 0, tmp, ind = 0;
  while(curr > prev)
  {
    tmp = curr;
    curr = hr_dat(++ind, lat.dim());
    prev = tmp;
  }
  hamSize = prev;
}

TightBinding::TightBinding(TightBinding &other)
{
  seedname = other.seedname;
  lat = other.lat;
  hr_dat = other.hr_dat;
  hr_weights = other.hr_weights;
  wsvec_dat = other.wsvec_dat;
  wsvec_weights = other.wsvec_weights;
  hamSize = other.hamSize;
}

/********** Methods from Lattice **********/

mat
TightBinding::latVecs()
{
  return lat.latticeVectors();
}

mat
TightBinding::kVecs()
{
  return lat.kVectors();
}

cube
TightBinding::kPoints()
{
  return lat.kPoints();
}

std::vector<std::string>
TightBinding::kPt_names_from()
{
  return lat.kPt_names_from();
}

std::vector<std::string>
TightBinding::kPt_names_to()
{
  return lat.kPt_names_to();
}


/********* Methods **********/
cx_mat
TightBinding::Ham(vec k)  //k in cartesian coordinates
{
  int                     latDim = lat.dim(),
                          rowIndex,
                          colIndex,
                          wsIndex = 0,
                          a, b, ind, nind,
                          site;
  vec                     coeffs(latDim);
  cx_mat                  H(hamSize, hamSize);
  std::complex<double>    hoppingEnergy, curr, weightedEnergy; 
                          curr;
  double                  cutoff = .0005;
  
  H.zeros();
  
  for(int i = 0; i < hr_dat.n_rows; i++)
  {
    rowIndex = hr_dat(i, latDim + 1) - 1;
    colIndex = hr_dat(i, latDim) - 1;
    site = i / (hamSize * hamSize);
    ind = i % (hamSize * hamSize);
    a = ind / hamSize;
    b = ind % hamSize;
    nind = hamSize * b + a + site * (hamSize * hamSize);
    hoppingEnergy = std::complex<double>(hr_dat(nind, latDim + 2), hr_dat(nind, latDim + 3));
    
    if(abs(hoppingEnergy) > cutoff)
    {
      hoppingEnergy = myRound(hoppingEnergy, cutoff);
    }
    else
    {
      hoppingEnergy = std::complex<double>(0, 0);
    }
    
    for(int j = 0; j < latDim; j++)
    {
      coeffs(j) = hr_dat(nind, j);
    }
    
    curr = 0;
    weightedEnergy = hoppingEnergy / (wsvec_weights(i) * hr_weights(i / pow(hamSize, 2)));
    for(int j = 0; j < wsvec_weights(i); j++)
    {
      curr += cexp(dot(k, latVecs() * (coeffs + trans(wsvec_dat.row(wsIndex++)))));
    }
    H(rowIndex, colIndex) += curr * weightedEnergy;
  }
     
  return H;
}

cx_cube
TightBinding::expandHam_order1(vec k)  //k in cartesian coordinates
{
  int                     latDim = lat.dim(),
                          rowIndex,
                          colIndex,
                          wsIndex = 0,
                          a, b, ind, nind,
                          site;
  vec                     coeffs(latDim),
                          R;
                          //k = k0 * (1 + .0001);
  cx_vec                  curr(latDim);
  cx_cube                 H_1(hamSize, hamSize, latDim);
  std::complex<double>    hoppingEnergy, wf_multiplier, weightedEnergy;
  double                  cutoff = .0005;
  
  H_1.zeros();
    
  for(int i = 0; i < hr_dat.n_rows; i++)
  {
    rowIndex = hr_dat(i, latDim + 1) - 1;
    colIndex = hr_dat(i, latDim) - 1;
    site = i / (hamSize * hamSize);
    ind = i % (hamSize * hamSize);
    a = ind / hamSize;
    b = ind % hamSize;
    nind = hamSize * b + a + site * (hamSize * hamSize);
    hoppingEnergy = std::complex<double>(hr_dat(nind, latDim + 2), hr_dat(nind, latDim + 1));
      
    if(abs(hoppingEnergy) > cutoff)
    {
      hoppingEnergy = myRound(hoppingEnergy, cutoff);
    }
    else
    {
      hoppingEnergy = std::complex<double>(0, 0);
    }
      
    for(int j = 0; j < latDim; j++)
    {
      coeffs(j) = hr_dat(nind, j);
    }
      
    curr.zeros();
    weightedEnergy = hoppingEnergy / (wsvec_weights(i) * hr_weights(i / pow(hamSize, 2)));
    for(int j = 0; j < wsvec_weights(i); j++)
    {
      R = latVecs() * (coeffs + trans(wsvec_dat.row(wsIndex++)));
      wf_multiplier = cexp(dot(k, R)) * weightedEnergy;
      H_1.tube(rowIndex, colIndex) += wf_multiplier * I * R;
    }
  }
  
  return H_1;
}

cx_cube
TightBinding::expandHam_order2(vec k)
{
  if(lat.dim() != 3)
  {
    throw "method TightBinding::expandHam : lattice must be of dimension 3";
  }
  int                     latDim = lat.dim(),
                          rowIndex,
                          colIndex,
                          wsIndex = 0,
                          a, b, ind, newInd,
                          site;
  vec                     coeffs(latDim),
                          R;
                          //k = k0 * (1 + .0001);
  cx_vec                  curr(latDim);
  cx_cube                 H_2(hamSize, hamSize, pow(latDim, 2));
  std::complex<double>    hoppingEnergy, wf_multiplier, weightedEnergy;
  double                  cutoff = .0005;
  
  H_2.zeros();
    
  for(int i = 0; i < hr_dat.n_rows; i++)
  {
    rowIndex = hr_dat(i, latDim + 1) - 1;
    colIndex = hr_dat(i, latDim) - 1;
    site = i / (hamSize * hamSize);
    ind = i % (hamSize * hamSize);
    a = ind / hamSize;
    b = ind % hamSize;
    newInd = hamSize * b + a + site * (hamSize * hamSize);
    hoppingEnergy = std::complex<double>(hr_dat(newInd, latDim + 2), hr_dat(newInd, latDim + 1));
      
    if(abs(hoppingEnergy) > cutoff)
    {
      hoppingEnergy = myRound(hoppingEnergy, cutoff);
    }
    else
    {
      hoppingEnergy = std::complex<double>(0, 0);
    }
      
    for(int j = 0; j < latDim; j++)
    {
      coeffs(j) = hr_dat(newInd, j);
    }
      
    curr.zeros();
    weightedEnergy = hoppingEnergy / (wsvec_weights(i) * hr_weights(i / pow(hamSize, 2)));
    for(int j = 0; j < wsvec_weights(i); j++)
    {
      R = latVecs() * (coeffs + trans(wsvec_dat.row(wsIndex++)));
      wf_multiplier = cexp(dot(k, R)) * weightedEnergy;
      
      for(int m = 0; m < latDim; m++)
      {
	for(int n = 0; n < latDim; n++)
	{
	  H_2(rowIndex, colIndex, m * latDim + n) += wf_multiplier * R(m) * R(n);
	}
      }
    }
  }
  
  return H_2;  
}


int
TightBinding::computeBands()
{
  std::string     path = "../data/" + seedname;
  std::ofstream   EnergyOut (path + "_bands.dat"),
                  GnuOut (path + "_bands.gnu");
  mat             kPointsFrom = kVecs() * kPoints().slice(0),
                  kPointsTo   = kVecs() * kPoints().slice(1);
  std::vector<std::string> kPtNamesFrom = kPt_names_from(),
                  kPtNamesTo = kPt_names_to();
  unsigned        numOfKPts = kPointsTo.n_cols;
  int             lineDensity = 600 / (numOfKPts - 1), 			
                  numOfRows = 0,
                  numOfCols = 0;
  double          pointCount = 0, 	//keeps track of how many points have been plotted in total.
                  sliceSize,			//keeps track of current slice of k-space.
                  maxEnergy = 0,
                  minEnergy = 0;
  vec             symPoints(numOfKPts);
    
  std::cout << "Calculating energy eigenvalues\n" << "...\n" << std::endl;
    
  for(int currentPoint = 1; currentPoint < numOfKPts; currentPoint++)
  {
    vec         pathToNext = kPointsTo.col(currentPoint) - kPointsFrom.col(currentPoint - 1),
                pathFrom = kPointsFrom.col(currentPoint - 1),
                k,
                energies;
    double      mult;
       
    sliceSize = norm(pathToNext);
    pointCount += sliceSize;
    for(int point = (pointCount - sliceSize) * lineDensity; point <= pointCount * lineDensity; point++)
    {
      mult = (double)(point - (pointCount - sliceSize) * lineDensity) / (lineDensity * (sliceSize));
      k = pathFrom + mult * pathToNext;
            
      cx_mat H = Ham(k);
      eig_sym( energies, H );
            
      for(int eigNum = 0; eigNum < energies.n_rows; eigNum++)
      {
        if (energies(eigNum) > maxEnergy) maxEnergy = energies(eigNum);
        if (energies(eigNum) < minEnergy) minEnergy = energies(eigNum);
        EnergyOut << std::setprecision(16) << (double)(point) / lineDensity
                  << "      " << energies(eigNum) << std::endl;
      }
    }
    symPoints(currentPoint - 1) = pointCount - sliceSize;
  }
  symPoints(numOfKPts - 1) = pointCount;
    
    /************************** Setting Output ***************************/
    
  std::cout << "Writing Output to file" << path << "_bands.dat" << "\n" << "...\n" << std::endl;
    
  GnuOut << "set terminal png" << '\n' << "set output '" << seedname << "_bands.png'" << std::endl;
  GnuOut << "unset arrow" 			<< std::endl;
  GnuOut << "set style data dots"	 	<< std::endl;
  GnuOut << "set nokey" 				<< std::endl;
  for(int currentPoint = 1; currentPoint < numOfKPts; currentPoint++)
  {
    GnuOut << "set arrow from "
           << symPoints(currentPoint - 1)
           << ",  " << minEnergy - 1
           << " to "
           << symPoints(currentPoint - 1) << ",  "
           << maxEnergy + 1 << " nohead" << std::endl;
  }
  GnuOut << "set xtics (";
  for(int curr = 0; curr < numOfKPts - 1; curr++)
  {
    GnuOut << "\" " << kPtNamesFrom[curr] << " \"  " << symPoints(curr) << ", ";
  }
  GnuOut <<  "\" "
         << kPtNamesTo[numOfKPts - 2]
         << "\"  " << symPoints(numOfKPts - 1)
         << ")" << std::endl;
  GnuOut << "set xrange [0: " << pointCount << "]" 						  << std::endl;
  GnuOut << "set yrange [" << minEnergy - 1 << ": " << maxEnergy + 1 << "]" << std::endl;
  GnuOut << "plot '" << seedname << "_bands.dat'" << std::endl;
  GnuOut << "set terminal xterm" << std::endl;
  GnuOut << "plot '" << seedname << "_bands.dat'";

  return 1;
}

/*
double Hartman(vec x)
{
  if(x.size() != 3)
  {
    throw "Hartman function input must be 3-vector.";
  }
    
  vec alpha(4);
  mat A(4, 3),
  P(4, 3);
    
  alpha(0) = 1;
  alpha(1) = 1.2;
  alpha(2) = 3;
  alpha(3) = 3.2;
    
  A << 3 << 10 << 30 << endr
    << .1 << 10 << 35 << endr
    << 3 << 10 << 30 << endr
    << .1 << 10 << 35 << endr;
    
  P << 3689 << 1170 << 2673 << endr
    << 4699 << 4387 << 7470 << endr
    << 1091 << 8732 << 5547 << endr
    << 381 << 5743 << 8828 << endr;
  P *= pow(10, -4);
    
  double  r = 0,
          s;
  for(int i = 0; i < 4; i++)
  {
    s = 0;
    for(int j = 0; j < 3; j++)
    {
      s += A(i, j) * pow(x(j) - P(i, j), 2);
    }
    r += alpha(i) * exp(-s);
  }
  return -r;
}
*/

double
TightBinding::bandGap(vec k) //k in cartesian coordinates
{
  vec         energies;
  
  cx_mat      H = Ham(k);
  int         len;
  double      rVal = 0;

  eig_sym( energies, H );
  energies = mergeSort(energies);
  
  //H.print();
  //energies.print();
  //std::cout << std::endl;
  
  len = energies.size();
  rVal = energies(len / 2) - energies(len / 2 - 1);
  return rVal;
}


vec
TightBinding::locateWeylNodes(vec k) //w in lattice coordinates
{
  if(k.size() != lat.dim())
  {
    throw "class \"TightBinding\", method \"findWeyl(vec w)\": Input vector must be of same dimension as lattice";
  }
    
  int     n = 3,
          iter = 0;
  mat     x(n, n + 1),
          f(n + 1, 2),
          P(n + 1, n + 1);
  vec     x_new(n),
          centroid(n),
          x_r(n),
          x_e(n),
          x_c(n),
          x_worst(n),
          w(n);
  double  f_new, f_r, f_e, f_c, f_best, f_worst;
  
  //Initiate vertices of simplex
  x.zeros();
  x = buildSimplex(kVecs() * k, .005);  //puts k in cartesian coordinates
  
    
  //evaluate function at initial vertices
  for(int i = 0; i < n + 1; i++)
  {
    f(i, 0) = bandGap(x.col(i));
    f(i, 1) = i; //To keep track of initial indices
  }
        
  f = mergeSort(f); //Sort by function value
  P.zeros();
  for(int i = 0; i < n + 1; i++)
  {
    P(f(i, 1), i) = 1;
  }
  x = x * P;
  f_best = f(0, 0);
  f_worst = f(n, 0);
    
  std::cout << "Attempting to minimize bandgap and Locate Weyl Nodes"
  << "\n...\n" << std::endl;
    
  while((simplexSize(x) > pow(10, -14) || (f_worst - f_best) > pow(10, -14)) && iter < 1000)
  {
    //Output
    if(!(iter % 10))
    {
      std::cout << "Iteration #:   " << iter << std::endl;
      std::cout << "Simplex Size:  " << std::setprecision(16)
                << simplexSize(x) << std::endl;
      std::cout << "Minimum Gap:   " << std::setprecision(16)
                << f(0, 0) << "\n" << std::endl;
      std::cout << "Simplex Vertices:    " << "\n";
      trans(x).print();
      std::cout << "Function at Vertices " << "\n";
      trans(f.col(0)).print();
      std::cout << "\n" << std::endl;
    }
    iter++;
    
    // 2) Reflection Step
    centroid.zeros();
    for(int i = 0; i < n; i++)
    {
      centroid += x.col(i);
    }
            
    centroid /= (double)n;
    x_r = centroid + (centroid - x.col(n));
    f_r = bandGap(x_r);
        
    if(f(0, 0) <= f_r && f_r < f(n - 1, 0))
    {
      x.col(n) = x_r;
      f(n, 0) = f_r;
    }
    else if(f_r < f(0, 0)) // 3) Expansion step
    {
      x_e = centroid + (2 * (x_r - centroid));
      f_e = bandGap(x_e);
      if(f_e < f_r)
      {
        x.col(n) = x_e;
        f(n, 0) = f_e;
      }
      else
      {
        x.col(n) = x_r;
        f(n, 0) = f_r;
      }
    }
    else if (f_r >= f(n-1, 0)) // 4) Contraction Step
    {
      if(f_r < f(n, 0))
      {
        x_worst = x.col(n);
        f_worst = f(n, 0);
      }
      else
      {
        x_worst = x_r;
        f_worst = f_r;
      }
            
      x_c = centroid + (.5 * (x.col(n) - centroid));
      f_c = bandGap(x_c);
            
      if(f_c < f_worst)
      {
        x.col(n) = x_c;
        f(n, 0) = f_c;
      }
      else
      {
        // 5) Shrink step
        for(int i = 1; i < n + 1; i++)
        {
          x.col(i) = x.col(0) + (.5 * (x.col(i) - x.col(0)));
          f(i, 0) = bandGap(x.col(i));
        }
      }
    }
        
    //Sorting vertices by function value
    for(int i = 0; i < n + 1; i++)
    {
      f(i, 1) = i;
    }
    f = mergeSort(f);
    P.zeros();
    for(int i = 0; i < n + 1; i++)
    {
      P(f(i, 1), i) = 1;
    }
        
    x = x * P;
    f_best = f(0, 0);
    f_worst = f(n, 0);

  }
  
  w = x.col(0);
  std::cout << "Band minimum " << f(0, 0) << "." << std::endl;
  std::cout << "Iterations taken: " << iter << "\n" << std::endl;
  return w;
  
}


cx_mat
TightBinding::fermiVelocity(vec k0)  //k in lattice coordinates
{
  vec       k = kVecs() * k0,
            energies;
  cx_mat    H_0 = Ham(k),
            U;
  cx_cube   H_1 = expandHam_order1(k);
  
  eig_sym(energies, U, H_0);
  
  /*
  eigVals.print();
  std::cout << "\n\n" << std::endl;
  (U.t() * H_0 * U).print();
  */
  
  for(int i = 0; i < lat.dim(); i++)
  {
    H_1.slice(i) = U.t() * H_1.slice(i) * U;
    //H_1.slice(i).print();
  }
  
  //Downfolding
  int     val = 15,
          cond = 16,
          dif = cond - val + 1;
  cx_cube H_eff(dif, dif, lat.dim());
  vec     eigs(dif);

  for(int i = 0; i < dif; i++)
  {
    for(int j = 0; j < dif; j++)
    {
      H_eff.tube(i, j) = H_1.tube(i + val, j + val);
    }
  }
  
  for(int i = 0; i < lat.dim(); i++)
  {
    //H_eff.slice(i) = downFold(H_1.slice(i), val, cond);
    //H_eff.slice(i) = H00.slice(i) + T01.slice(i) * inv(H11.slice(i)) * T10.slice(i);
    //H_eff.slice(i).print();
    eig_sym(eigs, H_eff.slice(i));
    eigs.print();
    std::cout << std::endl;
  }
  
  return H_0;
  
}



//could also do monte-carlo integration
vec
TightBinding::injectionCurrent(double omega, vec A, double T)
{
  if(lat.dim() != 3)
  {
    throw "method TightBinding::phCurrent : lattice must be of dimension 3";
  }
  
  std::cout << "Attempting to calculate photocurrent."
  << "\n...\n" << std::endl;

  
  cx_cube               H_1;
  vec                   k(lat.dim()),
                        energies,
                        sum(k.size()),
                        tmp = k,
                        grad(k.size());
  int                   slices = 4;
  double                delta,
                        Ef = 15.2886, //eV
                        alpha = 25,
                        volume = std::abs(det(kVecs())),
                        f_l, f_s, h = .00001;
  std::complex<double>  V = 0;
  
  //define mesh in k-space
  
  for(int i = 1; i <= slices; i++)
  {
    for(int j = 1; j <= slices; j++)
    {
      for(int l = 1; l <= slices; l++)
      {
        if(!(i - 1 % 10) && j - 1 == 0 && l - 1 == 0)
        {
          std::cout << (i - 1) * pow(slices, 2) + (j - 1) * slices + l - 1 << " out of " << pow(slices, 3) << std::endl;
        }
        
        k << i << j << l;
        k = kVecs() * k / slices;
        
        eig_sym(energies, Ham(k));
        
        //obtain deriv of bandGap using numerical approximation
        for(int m = 0; m < k.size(); m++)
        {
          tmp(m) += h;
          f_l = bandGap(tmp);
          tmp(m) -= 2 * h;
          f_s = bandGap(tmp);
          grad(m) = (f_l - f_s);    //Second order error
        }
        
        //obtain transition amplitudes/matrix elements
        H_1 = expandHam_order1(k);
      
        for(int m = 0; m < A.size() * (A.size() < lat.dim()) +  lat.dim() * (lat.dim() <= A.size()); m++)
        {
          V += H_1(16, 15, m) * A(m);
        }

        //Use narrow Gaussian as delta function
        delta = exp(std::pow(alpha * (bandGap(k) - hBar * omega), 2));
        
        sum += grad * norm(V) * norm(V) * delta * (fermiDirac(energies(15), Ef, T) -fermiDirac(energies(16), Ef, T));
      }
    }
  }
  std::cout << std::endl;
  return sum * volume / (pow(slices, lat.dim()) * hBar * h * 2);
  
  //need to convert units
}

cx_vec
TightBinding::shiftCurrent(double omega, vec A, double T)
{
  if(lat.dim() != 3)
  {
    throw "method TightBinding::phCurrent : lattice must be of dimension 3";
  }
  
  std::cout << "Attempting to calculate photocurrent."
  << "\n...\n" << std::endl;
  
  int                   slices = 2, r_flag, r_k_flag, latDim = lat.dim();
  cx_cube               v, H_2,
                        r(hamSize, hamSize, latDim),
                        r_k(hamSize, hamSize, latDim),
                        d(hamSize, hamSize, latDim),
			w(hamSize, hamSize, latDim * latDim),
                        susceptibilityTensor(latDim, latDim, latDim);
  vec                   k(latDim),
                        energies;
  cx_vec                current(latDim);
  cx_mat                H, U;
  double                del_mn, del_nm, E_mn, r_mn,
                        E_f = 15.2886, //eV
                        alpha = 10,
                        dV = std::abs(det(kVecs())) * pow(slices + ((slices + 1) % 2), -latDim),
                        f_mn, w_mn, cutoff = .0000000000000001;
  std::complex<double>  susceptibilityTerm;
  
  susceptibilityTensor.zeros();
  
  //define mesh in k-space
  for(int k1 = -slices / 2; k1 <= slices / 2; k1++)
  {
    for(int k2 = -slices / 2; k2 <= slices / 2; k2++)
    {
      for(int k3 = -slices / 2; k3 <= slices / 2; k3++)
      {
        
        k << k1 << k2 << k3;
        k /= slices + 1;
        //k.print();
	//std::cout << std::endl;
	k = -kVecs() * k;
        H = Ham(k);

        eig_sym(energies, U, H);
        r_flag = 1;
        
        for(int m = 0; m < hamSize; m++)
        {
          for(int n = 0; n < hamSize; n++)
          {
            f_mn = fermiDirac(energies(m), E_f, T) - fermiDirac(energies(n), E_f, T);
            
            if(std::abs(f_mn) > cutoff)
            {
	      w_mn = (energies(m) - energies(n)) / hBar; 
	      if(w_mn == 0) std::cout << m << "  " << n << "  " << w_mn << std::endl;	
              del_mn = delta(hBar * (w_mn - omega), alpha);
              if(del_mn > cutoff)
              {
                if(r_flag) //builds v, r, d, w matrices
                {
                  
                  v = expandHam_order1(k);
		  w = expandHam_order2(k);
		 
                  for(int i = 0; i < latDim; i++)
                  {
                    v.slice(i) = U.t() * v.slice(i) * U;
		    for(int j = 0; j < latDim; j++)
		    {
		      w.slice(i * latDim + j) = U.t() * w.slice(i * latDim + j) * U;
		    }
                  }
                  
                  for(int p = 0; p < hamSize; p++)
                  {
                    for(int q = 0; q < hamSize; q++)
                    {
		      if(p != q)
		      {
			r.tube(p, q) = -v.tube(p, q) * I / (energies(p) - energies(q));
		      }
		      else
		      {
			r.tube(p, q).zeros();
		      }	
		      d.tube(p, q) = v.tube(p, p) - v.tube(q, q);
                    }
                  }
                  r_flag = 0;
                }
                
                //calculate contribution to rank 3 tensor at each k, m, n
                for(int a = 0; a < latDim; a++)
                {
                  r_k_flag = 1;
                  for(int b = 0; b < latDim; b++)
                  {
                    for(int c = 0; c < latDim; c++)
                    {
                      if(r_k_flag) //builds rk matrix
                      {
                        for(int i = 0; i < hamSize; i++)
                        {
                          for(int j = 0; j < hamSize; j++)
                          {
			    if(i != j)
			    {
			      for(int p = 0; p < latDim; p++)
                              {
                                r_k(i, j, p) = -(r(i, j, p) * d(i, j, a) + r(i, j, a) * d(i, j, p)); 
                                for(int q = 0; q < hamSize; q++)
                                {
                                  r_k(i, j, p) -= v(i, q, p) * r(q, j, a) - r(i, q, a) * v(q, j, p);
                                }
                                r_k(i, j, p) *= hBar / (energies(i) - energies(j));
				//r_k(i, j, p) -= w(i, j, p * latDim + a);
                              }
			    }
                            
                          }
                        }
			r_k.print();
                        r_k_flag = 0;
                      }
                      
		      susceptibilityTerm = f_mn * (r(m, n, b) * r_k(n, m, c) + r(m, n, c) 
							    * r_k(n, m, b)) * del_mn * dV;
                      susceptibilityTensor(a, b, c) += susceptibilityTerm;
                      current(a) += susceptibilityTerm;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  susceptibilityTensor.print();
  return current;
}


int
TightBinding::plotGap(int h, int k, int l) //integers represent miller indices
{
  mat             vertices(lat.dim(), 3);
  //Must find set of vertices in R^3 outlining our plane
  std::stringstream ss;
  ss << h << k << l;
  std::string     path = "../data/" + seedname + "_gap_" + ss.str() + ".dat";
  
  /*vertices << -0.72823 << -0.72823 << -.27176 << endr
             << .27176   << .27176   << 0.72823 << endr
             << 0.72823  << 0.72823  << .27176  << endr;*/
    
  vertices << 0 << -.5 << 0 << endr
           << .5 << 0 << 0 << endr
           << 0 << .5 << -.5 << endr;
    
  vertices = trans(3 * vertices);
    
  field<vec>      pts = grid(vertices, 200);
  std::ofstream   ofs(path);

  int     ht = pts.n_rows,
          wid = pts.n_cols,
          len = pts.n_slices;
    
  std::cout << "Calculating bandgap along " + ss.str() + " plane.\n..." << std::endl;
  for(int k = 0; k < len; k++)
  {
    for(int i = 0; i < wid; i++)
    {
      for(int j = 0; j < ht; j++)
      {
        ofs << i + 1 << "      " << j + 1 << "      "
            << bandGap(pts(i, j, k)) << std::endl;
      }
    }
  }
  return 1;
}


