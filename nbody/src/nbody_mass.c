/*
 * Copyright (c) 2012 Rensselaer Polytechnic Institute
 * Copyright (c) 2016-2018 Siddhartha Shelton
 * 
 * This file is part of Milkway@Home.
 *
 * Milkyway@Home is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Milkyway@Home is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Milkyway@Home.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "nbody_mass.h"
#include "nbody_defaults.h"
#include "milkyway_math.h"
#include "nbody_types.h"


// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// There functions are involved in calculating a binomial distribution
/*In order to decrease the size of the numbers
 * computed all these functions are
 * calculated in log space*/
static real factorial(int n)
{
     int counter;
     real result = 0.0;

     for (counter = n; counter >= 1; counter--)
       {
          result += mw_log((real) counter);
       }

     return result;
}


static real choose(int n, int c)
{
    unsigned int i;
    real result = 0.0;
    
    /* This for loop calulates log(n!/(n-c)!) */
    for (i = n - c + 1; i <= (unsigned int) n; ++i)
    {
        result += mw_log(i);
    }
    result -= factorial(c);
    return result;
}

real probability_match(int n, real ktmp, real pobs)
{
    real result = 0.0;

    /*
     * Previously, this function took in k as an int. Bad move.
     * This function was called twice, one of which sent a real valued k: (int) k1 and (real) k2
     * That real k2 was then converted to int. Could result in converted (int) k1 != (int) k2 when k1 = k2. 
     * Special result was poor likelihood for some histograms when check against themselves!
     * General results: unknown. But probably not good. (most likely caused different machines to report
     * different likelihood values).
     * 
     */
    int k = (int) mw_round(ktmp);    //patch. See above. 
    //The previous calculation does not return the right values.  Furthermore, we need a zeroed metric.                                                                                              
    result =  (real) choose(n, k);
    result += k * mw_log(pobs); 
    result += (n - k) * mw_log(1.0 - pobs);
    
    
    return mw_exp(result);
}
// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 
// IMPLEMENTATION OF GAMMA FUNCTIONS. COMPLETE AND INCOMPLETE
real GammaFunc(const real z) 
{
    //Alogrithm for the calculation of the Lanczos Approx of the complete Gamma function 
    //as implemented in Numerical Recipes 3rd ed, 2007.
    real g = 4.7421875; //g parameter for the gamma function
    real x, tmp, y, A_g;
    
    //these are the cn's
    static const real coeff[14] = {57.1562356658629235,-59.5979603554754912,
                                14.1360979747417471,-0.491913816097620199,.339946499848118887e-4,
                                .465236289270485756e-4,-.983744753048795646e-4,.158088703224912494e-3,
                                -.210264441724104883e-3,.217439618115212643e-3,-.164318106536763890e-3,
                                .844182239838527433e-4,-.261908384015814087e-4,.368991826595316234e-5};
    y = x = z;
    tmp = x + g + 0.5;
    tmp = (x + 0.5) * mw_log(tmp) - tmp;
    A_g = 0.999999999999997092; //this is c0
    
    for (int j = 0; j < 14; j++) 
    {
        A_g += coeff[j] / ++y;
    } //calculates the series approx sum
        
    //sqrt(2 * pi) = 2.5066282746310005
    tmp += mw_log(2.5066282746310005 * A_g / x);//returns the log of the gamma function
    
    return mw_exp(tmp);
}

static real shift_factorial(real z, real n)
{
    int counter;
    real result = 0.0;
    for(counter = n; counter >= 1; counter--)
    {
        result += mw_log(z + (real) counter);
    }
    return mw_exp(result);
    
    
}


static real series_approx(real a, real x)
{

    real sum, del, ap;
    real pow_x = 1.0;
//     ap = a;
    ap = 0.0;
    del = sum = 1.0 / a;//starting: gammma(a) / gamma(a+1) = 1/a
    for (;;) 
    {
//         ++ap;
        ++ap;
//         del *= x / ap;

        pow_x *= x;
        del = pow_x / (a * shift_factorial(a, ap));
        
        sum += del;
        if (mw_fabs(del) < mw_fabs(sum) * 1.0e-15) 
        {
            return sum * exp(-x + a * log(x));
        }
    }
    
}
                            
real IncompleteGammaFunc(real a, real x)
{
    //the series approx returns gamma from 0 to X but we want from X to INF
    //Therefore, we subtract it from GammaFunc which is from 0 to INF
    //The continued frac approx is already from X to INF
    
//     static const real max_a = 100;
    real gamma = GammaFunc(a);

    if (x == 0.0) return gamma;
    // Use the series representation. 
    return gamma - series_approx(a,x);
    

}    

// // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // // 

real calc_vLOS(const mwvector v, const mwvector p, real sunGCdist)
{
    real xsol = X(p) + sunGCdist;
    real mag = mw_sqrt( xsol * xsol + Y(p) * Y(p) + Z(p) * Z(p) );
    real vl = xsol * X(v) + Y(p) * Y(v) + Z(p) * Z(v);
    vl = vl / mag;
    
    return vl;
}

real calc_distance(const mwvector p, real sunGCdist)  /**Calculating the distance to each body **/
{
    real xsol = X(p) + sunGCdist;
    real distance = mw_sqrt(xsol * xsol + Y(p) * Y(p) + Z(p) * Z(p) );

    return distance;
}

/* Get the dispersion in each bin*/
void nbCalcDisp(NBodyHistogram* histogram, mwbool initial, real correction_factor)
{
    unsigned int i;
    unsigned int j;
    unsigned int Histindex;
    
    unsigned int lambdaBins = histogram->lambdaBins;
    unsigned int betaBins = histogram->betaBins;
    
    HistData* histData = histogram->data;

    real count;
    real n_ratio;
    real n_new;
    real sum, sq_sum, dispsq;
    
    
    for (i = 0; i < lambdaBins; ++i)
    {
        for(j = 0; j < betaBins; ++j)
        {
            Histindex = i * betaBins + j;
            count = (real) histData[Histindex].rawCount;
            count -= histData[Histindex].outliersRemoved;
            
            if(count > 10.0)//need enough counts so that bins with minimal bodies do not throw the vel disp off
            {
                n_new = count - 1.0; //because the mean is calculated from the same populations set
                n_ratio = count / (n_new); 
                
                sq_sum = histData[Histindex].sq_sum;
                sum = histData[Histindex].sum;
                
                vdispsq = (sq_sum / n_new) - n_ratio * sqr(sum / count);
                
                /* The following requires explanation. For the first calculation of dispersions, the bool initial 
                 * needs to be set to true. After that false.
                 * It will correct if there was no outliers removed because then the distribution does not have wings
                 * It will also correct if outliers were removed because then the wings were removed. 
                 * Does one correction everytime there was an outlier removed. Corrects once if no outliers were removed. 
                 */
                
                if(!initial)
                {
                    dispsq *= correction_factor;
                }//correcting for truncating the distribution when removing outliers.

                histData[Histindex].variable = mw_sqrt(dispsq);
                histData[Histindex].err =  mw_sqrt( (count + 1) /(count * n_new ) ) * histData[Histindex].variable ;
                
            }
        }
    }
    
}


void nbRemoveOutliers(const NBodyState* st, NBodyHistogram* histogram, real * use_body, real * var, real sigma_cutoff, real sunGCdist)
{
    
    unsigned int Histindex;
    Body* p;
    HistData* histData;
    const Body* endp = st->bodytab + st->nbody;

    unsigned int counter = 0;
    
    histData = histogram->data;

    real v_line_of_sight;
    real distance;
    real bin_ave, bin_sigma, new_count;
    
    for (p = st->bodytab; p < endp; ++p)
    {
        /* Only include bodies in models we aren't ignoring */
        if (!ignoreBody(p))
        {
            
            /* Check if the position is within the bounds of the histogram */
            if (use_body[counter] >= 0)//if its not -1 then it was in the hist and set to the Histindex   
            {   
                Histindex = (int) use_body[counter];
                
                this_var = var[counter];
                /* bin count minus what was already removed */
                new_count = ((real) histData[Histindex].rawCount - histData[Histindex].outliersRemoved);
                
                /* average bin vel */
                bin_ave = histData[Histindex].v_sum / new_count;
                
                /* the sigma for the bin is the same as the dispersion */
                bin_sigma = histData[Histindex].variable;
                
                if(mw_fabs(bin_ave - this_var) > sigma_cutoff * bin_sigma)//if it is outside of the sigma limit
                {
                    histData[Histindex].sum -= v_line_of_sight;//remove from vel dis sums
                    histData[Histindex].sq_sum -= sqr(this_var);
                    histData[Histindex].outliersRemoved++;//keep track of how many are being removed
                    use_body[counter] = DEFAULT_NOT_USE;//marking the body as having been rejected as outlier

                    /** Adjusting the distance parameters as well **/
                    distance = calc_distance(Pos(p), sunGCdist);
                    histData[Histindex].avgDistance -= distance;
                }

                
            }
            counter++;
        }
    }
    
    
}


real nbCostComponent(const NBodyHistogram* data, const NBodyHistogram* histogram)
{
    unsigned int n = histogram->totalSimulated;
    unsigned int nSim = histogram->totalNum;
    unsigned int nData = data->totalNum;
    real histMass = histogram->massPerParticle;
    real dataMass = data->massPerParticle;
    real p; /* probability of observing an event */

    if (data->lambdaBins != histogram->lambdaBins || data->betaBins != histogram->betaBins)
    {
        /* FIXME?: We could have mismatched histogram sizes, but I'm
        * not sure what to do with ignored bins and
        * renormalization */
        return NAN;
    }

    if (nSim == 0 || nData == 0)
    {
        /* If the histogram is totally empty, it is worse than the worst case */
        return INFINITY;
    }

    if (histMass <= 0.0 || dataMass <= 0.0)
    {
        /*In order to calculate likelihood the masses are necessary*/
        return NAN;
    }
    
    /* this is the newest version of the cost function
     * it uses a combination of the binomial error for sim 
     * and the poisson error for the data
     */
    
    p = ((real) nSim / (real) n) ;
    real num = - sqr(dataMass * (real) nData - histMass * (real) nSim);
    real denom = 2.0 * (sqr(dataMass) * (real) nData + sqr(histMass) * (real) nSim * p * (1.0 - p));
    real CostComponent = num / denom; //this is the log of the cost component

    /* the cost component is negative. Returning a postive value */
    return -CostComponent;
    
}

/* for use with velocity dispersion, beta dispersion, average vlos,
average beta, and average distance likelihood component calculations */
real nbLikelihood(const NBodyHistogram* data, const NBodyHistogram* histogram)
{
    unsigned int lambdaBins = data->lambdaBins;
    unsigned int betaBins = data->betaBins;
    unsigned int nbins = lambdaBins * betaBins;
    real Nsigma_sq = 0.0;
    real Data;
    real Hist;
    real err_data, err_hist;
    real probability;
    for (unsigned int i = 0; i < nbins; ++i)
    {
        if (data->data[i].useBin)
        {
            Data = data->data[i].variable;
            /* the data may have incomplete information. Where it does not have will have -1 */
            if(Data > 0)
            {
                
                err_data = data->data[i].err;
                err_hist = histogram->data[i].err;
                
                Hist = histogram->data[i].variable;

                /* the error in simulation of variable is set to zero. */
                if(err_data == 0.0)
                {
                    //this should never actually end up running
                    Nsigma_sq += sqr( (Data - Hist) );
                }
                else
                {
                    Nsigma_sq += sqr( Data - Hist ) / ( sqr(err_data) + sqr(err_hist) );
                }
            }
        }

    }
        probability = (Nsigma_sq) / 2.0; //should be negative, but we return the negative of it anyway
    
    return probability;
}