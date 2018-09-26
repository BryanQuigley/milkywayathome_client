/*
 * Copyright (c) 2010, 2011 Matthew Arsenault
 * Copyright (c) 2010, 2011 Rensselaer Polytechnic Institute.
 * Copyright (c) 2016-2018 Siddhartha Shelton
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

#include "nbody_priv.h"
#include "nbody_potential.h"
#include "nbody_potential_types.h"
#include "milkyway_util.h"
#include "nbody_caustic.h"

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

/*Methods to be called by potentials*/

static inline real lnfact(int n)
{
     int counter;
     real result = 0.0;
     if (n > 0)
     {
          for (counter = n; counter >= 1; counter--)
          {
               result += mw_log((real) counter);
          }
     }
     /*mw_printf("ln(%u!) = %.15f \n",n,result);*/
     return result;
}

static inline real binom(int n, int k)
{
    return mw_exp(lnfact(n)-lnfact(k)-lnfact(n-k));
}

static inline real leg_pol(real x, int l)
{
    real sum = 0.0;
    for (int m = 0; m < mw_floor(l/2)+1; m++)
    {
        sum += mw_pow(-1, m)*mw_pow(x, l-2*m)/mw_pow(2,l)*binom(l,m)*binom(2*(l-m),l);
    }
    if (x == 0.0)
    {
        if (l*1.0/2.0 == mw_ceil(l*1.0/2.0))
        {
            sum = mw_pow(-1,l/2)*mw_exp(lnfact(l)-2*lnfact(l/2) - l*mw_log(2));
        }
        else
        {
            sum = 0.0;
        }
    }
    /*mw_printf("P(%.15f, %u) = %.15f \n",x,l,sum);*/
    return sum;
}

static inline real leg_pol_derv(real x, int l)
{
    real sum = 0.0;
    for (int m = 0; m < mw_floor((l-1)/2)+1; m++)
    {
        sum += mw_pow(-1, m)*mw_pow(x, l - 2*m - 1)/mw_pow(2,l)*binom(l,m)*binom(2*(l-m),l)*(l - 2*m);
    }
    /*mw_printf("P'(%.15f, %u) = %.15f \n",x,l,sum);*/
    return sum;
}

static inline real lower_gamma(int n, real x)
{
    real sum = 0;
    for (int k = 0; k < n; k++)
    {
       sum += mw_pow(x,k)/mw_exp(lnfact(k));
    }
    /*mw_printf("g(%u, %.15f) = %.15f \n",n,x,mw_exp(lnfact(n-1))*(1-mw_exp(-x)*sum));*/
    return mw_exp(lnfact(n-1))*(1-mw_exp(-x)*sum);
}

static inline real GenExpIntegral(int n, real x) /*Optimized for convergence at n=1,x=0.5*/
{
    return mw_exp(-x)/(x + n/(1+1/(x + (n+1)/(1 + 2/(x + (n+2)/(1 + 3/(x + (n+3)/(1 + 4/(x + (n+4)/(1 + 5/(x + (n+5)/(1 + 6/(x + (n+6)/(1 + 7/(x + (n+7)/(1 + 8/(x + (n+8)/(1 + 9/(x + (n+9)/11)))))))))))))))))));
}

static inline real besselI0(real x)
{
    real add = 0.0;
    for (int i = 0; i < 40; i++)
    {
        add += mw_pow(x/2.0,2*i)/mw_exp(2*lnfact(i));
    }
    return add;
}

static inline real besselI1(real x)
{
    real add = 0.0;
    for (int i = 0; i < 40; i++)
    {
        add += mw_pow(x/2.0,2*i + 1)/mw_exp(2*lnfact(i) + mw_log((real) i + 1.0));
    }
    return add;
}

static inline real besselK0(real x)
{
    const int n = 10;
    const real a = 0.0;
    const real b = 10.0;

    real integral = 0.0;
    const real weight[] = {5,8,5};
    const real point[] = {-0.774596669, 0 , 0.774596669};
    const real frac = 9.0;
    const real h = (b-a)/n;

    for (int i = 0; i < n; i++)
    {
        real piece = 0.0;
        for (int j = 0; j < 3; j++)
        {
            real w_val = h*point[j]/2 + a+(i+0.5)*h;
            real funcval = mw_exp(-2*x*mw_pow(mw_sinh(w_val/2),2));
            piece = piece + h*weight[j]*funcval/frac/2.0;
        }
        integral  = integral + piece;
    }
    //mw_printf("K0(%.15f) = %.15f\n",x,integral*mw_exp(-x));
    return integral*mw_exp(-x);
}

static inline real besselK1(real x)
{
    const int n = 10;
    const real a = 0.0;
    const real b = 15.0;

    real integral = 0.0;
    const real weight[] = {5,8,5};
    const real point[] = {-0.774596669, 0 , 0.774596669};
    const real frac = 9.0;
    const real h = (b-a)/n;

    for (int i = 0; i < n; i++)
    {
        real piece = 0.0;
        for (int j = 0; j < 3; j++)
        {
            real w_val = h*point[j]/2 + a+(i+0.5)*h;
            real funcval = mw_exp(-2*x*mw_pow(mw_sinh(w_val/2),2))*mw_cosh(w_val);
            piece = piece + h*weight[j]*funcval/frac/2.0;
        }
        integral  = integral + piece;
    }
    //mw_printf("K1(%.15f) = %.15f\n",x,integral*mw_exp(-x));
    return integral*mw_exp(-x);
}

static inline real aExp(real k, real R, real Rd)
{
    const int n = 5;
    const real a = 0.0;
    const real b = R;

    real integral = 0.0;
    const real weight[] = {5,8,5};
    const real point[] = {-0.774596669, 0 , 0.774596669};
    const real frac = 9.0;
    const real h = (b-a)/n;

    for (int i = 0; i < n; i++)
    {
        real piece = 0.0;
        for (int j = 0; j < 3; j++)
        {
            real x_val = h*point[j]/2 + a+(i+0.5)*h;
            real funcval = x_val*mw_exp(-x_val/Rd)*besselI0(k*x_val);
            piece = piece + h*weight[j]*funcval/frac/2.0;
        }
        integral  = integral + piece;
    }
    //mw_printf("aExp(%.15f,%.15f,%.15f) = %.15f\n",k,R,Rd,integral/sqr(Rd));
    return integral/sqr(Rd);
}

static inline real bExp(real k, real R, real Rd)
{
    const int n = 7;
    const real a = R;
    const real b = 15.0*R;

    real integral = 0.0;
    const real weight[] = {5,8,5};
    const real point[] = {-0.774596669, 0 , 0.774596669};
    const real frac = 9.0;
    const real h = (b-a)/n;

    for (int i = 0; i < n; i++)
    {
        real piece = 0.0;
        for (int j = 0; j < 3; j++)
        {
            real x_val = h*point[j]/2 + a+(i+0.5)*h;
            real funcval = x_val*mw_exp(-x_val/Rd)*besselK0(k*x_val);
            piece = piece + h*weight[j]*funcval/frac/2.0;
        }
        integral  = integral + piece;
    }
    //mw_printf("bExp(%.15f,%.15f,%.15f) = %.15f\n",k,R,Rd,integral/sqr(Rd));
    return integral/sqr(Rd);
}

static inline real RExpIntegrand (real k, real R, real Rd, real z, real zd)
{
    real val = k*mw_cos(k*z)*(aExp(k,R,Rd)*besselK1(k*R) - bExp(k,R,Rd)*besselI1(k*R))/(sqr(zd*k) + 1);
    //mw_printf("RExp(%.15f,%.15f,%.15f,%.15f,%.15f) = %.15f\n",k,R,Rd,z,zd,val);
    return val;
}

static inline real ZExpIntegrand (real k, real R, real Rd, real z, real zd)
{
    real val = k*mw_sin(k*z)*(aExp(k,R,Rd)*besselK0(k*R) + bExp(k,R,Rd)*besselI0(k*R))/(sqr(zd*k) + 1);
    //mw_printf("ZExp = %.15f\n",val);
    return val;
}

static inline real RSechIntegrand (real k, real R, real Rd, real z, real zd)
{
    real val = 3.1415926535*sqr(k)*zd/mw_sinh(3.1415926535*k*zd/4)/8.0*mw_cos(k*z)*(aExp(k,R,Rd)*besselK1(k*R) - bExp(k,R,Rd)*besselI1(k*R))/(sqr(zd*k) + 1);
    //mw_printf("RSech(%.15f,%.15f,%.15f,%.15f,%.15f) = %.15f\n",k,R,Rd,z,zd,val);
    return val;
}

static inline real ZSechIntegrand (real k, real R, real Rd, real z, real zd)
{
    real val = 3.1415926535*sqr(k)*zd/mw_sinh(3.1415926535*k*zd/4)/8.0*mw_sin(k*z)*(aExp(k,R,Rd)*besselK0(k*R) + bExp(k,R,Rd)*besselI0(k*R))/(sqr(zd*k) + 1);
    //mw_printf("ZSech = %.15f\n",val);
    return val;
}

/*spherical bulge potentials*/

static inline mwvector hernquistSphericalAccel(const Spherical* sph, mwvector pos, real r)
{
    const real tmp = sph->scale + r;

    return mw_mulvs(pos, -sph->mass / (r * sqr(tmp)));
}

static inline mwvector plummerSphericalAccel(const Spherical* sph, mwvector pos, real r)
{
    const real tmp = mw_sqrt(sqr(sph->scale) + sqr(r));

    return mw_mulvs(pos, -sph->mass / cube(tmp));
}

/* Disk Potentials */

static inline mwvector miyamotoNagaiDiskAccel(const Disk* disk, mwvector pos, real r)
{
    mwvector acc;
    const real a   = disk->scaleLength;
    const real b   = disk->scaleHeight;
    const real zp  = mw_sqrt(sqr(Z(pos)) + sqr(b));
    const real azp = a + zp;

    const real rp  = sqr(X(pos)) + sqr(Y(pos)) + sqr(azp);
    const real rth = mw_sqrt(cube(rp));  /* rp ^ (3/2) */

    X(acc) = -disk->mass * X(pos) / rth;
    Y(acc) = -disk->mass * Y(pos) / rth;
    Z(acc) = -disk->mass * Z(pos) * azp / (zp * rth);

    //mw_printf("Acceleration[AX,AY,AZ] = [%.15f,%.15f,%.15f]\n",X(acc),Y(acc),Z(acc));

    return acc;
}

/*WARNING: This potential uses incomplete gamma functions and the generalized exponential integral function. This potential will take longer than other potentials to run.*/
static inline mwvector freemanDiskAccel(const Disk* disk, mwvector pos, real r)     /*From Freeman 1970*/
{
    mwvector acc;
    /*Reset acc vector*/
    X(acc) = 0.0;
    Y(acc) = 0.0;
    Z(acc) = 0.0;

    const mwvector r_hat = mw_mulvs(pos, 1.0/r);
    const real r_proj = mw_sqrt(sqr(X(pos)) + sqr(Y(pos)));

    mwvector theta_hat;
    X(theta_hat) = X(pos)*Z(pos)/r/r_proj;
    Y(theta_hat) = Y(pos)*Z(pos)/r/r_proj;
    Z(theta_hat) = -r_proj/r;

    const real scl = disk->scaleLength;
    const real M = disk->mass;
    const real costheta = Z(pos)/r;
    const real sintheta = r_proj/r;

    real r_f = 0.0;
    real theta_f = 0.0;
    real a_r = 0.0;
    real b_r = 0.0;
    real p0 = 0.0;
    real pcos = 0.0;

    for (int l = 0; l < 5; l++) /*Only even terms add, so counter is multiplied by 2*/
    {
        if (l>0)
        {
            a_r = GenExpIntegral(2*l,r/scl);
        }
        else
        {
            a_r = 0.0;
        }
        b_r = mw_pow(scl/r,2*l+2)*lower_gamma(2*l+2,r/scl);
        p0 = leg_pol(0,2*l);
        pcos = leg_pol(costheta,2*l);
        r_f += p0*pcos*(2*l*a_r - (2*l+1)*b_r);

        if (l > 0)
        {
            theta_f -= p0*(a_r+b_r)*sintheta*leg_pol_derv(costheta,2*l);
        }
    }

    mwvector r_comp = mw_mulvs(r_hat, r_f*M/sqr(scl));
    mwvector theta_comp = mw_mulvs(theta_hat, theta_f*M/sqr(scl));

    X(acc) = X(r_comp) + X(theta_comp);
    Y(acc) = Y(r_comp) + Y(theta_comp);
    Z(acc) = Z(r_comp) + Z(theta_comp);

    if (r_f > 0)
    {
        mw_printf("ERROR: Repulsive acceleration!\n");
        mw_printf("r_magnitude = %.15f\n",r_f*M/sqr(scl));
        mw_printf("[X,Y,Z] = [%.15f,%.15f,%.15f]\n",X(pos),Y(pos),Z(pos));
        mw_printf("Acceleration = %.15f \n",mw_absv(acc));
    }

    return acc;
}

/*WARNING: This potential is EVEN SLOWER than the Freeman Potential! Do NOT use this for NBody!*/
static inline mwvector doubleExponentialDiskAccel(const Disk* disk, mwvector pos, real r)
{
    //mw_printf("Calculating Acceleration\n");
    //mw_printf("[X,Y,Z] = [%.15f,%.15f,%.15f]\n",X(pos),Y(pos),Z(pos));
    mwvector acc;

    const real R = mw_sqrt(sqr(X(pos)) + sqr(Y(pos)));
    mwvector R_hat;
    X(R_hat) = X(pos)/R;
    Y(R_hat) = Y(pos)/R;
    Z(R_hat) = 0.0;

    mwvector Z_hat;
    X(Z_hat) = 0.0;
    Y(Z_hat) = 0.0;
    Z(Z_hat) = 1.0;

    const real Rd = disk->scaleLength;
    const real zd = disk->scaleHeight;
    const real M = disk->mass;
    const real z = Z(pos);

    const int n = 15;
    const real a = 0.0;
    const real b = 60.0/R;
    real integralR = 0.0;
    real integralZ = 0.0;
    const real weight[] = {0.236927,0.478629,0.568889,0.478629,0.236927};
    const real point[] = {-0.90618,-0.538469,0.0,0.538469,0.90618};
    const real h = (b-a)/(n*1.0);

    for (int k = 0; k < n; k++)     /*Five-point Gaussian Quadrature*/
    {
        real Rpiece = 0.0;
        real Zpiece = 0.0;
        real k_val = 0.0;
        for (int j = 0; j < 5; j++)
        {
            k_val = h*point[j]/2 + a+(k*1.0+0.5)*h;
            Rpiece = Rpiece + h*weight[j]*RExpIntegrand(k_val,R,Rd,z,zd)/2.0;
            Zpiece = Zpiece + h*weight[j]*ZExpIntegrand(k_val,R,Rd,z,zd)/2.0;
        }
        integralR  = integralR + Rpiece;
        integralZ  = integralZ + Zpiece;
    }

    mwvector R_comp = mw_mulvs(R_hat, -2.0*M/3.1415926535*integralR);
    mwvector Z_comp = mw_mulvs(Z_hat, -2.0*M/3.1415926535*integralZ);

    X(acc) = X(R_comp) + X(Z_comp);
    Y(acc) = Y(R_comp) + Y(Z_comp);
    Z(acc) = Z(R_comp) + Z(Z_comp);

    //real magnitude = mw_sqrt(sqr(X(acc))+sqr(Y(acc))+sqr(Z(acc)));

    //mw_printf("Acceleration[AX,AY,AZ] = [%.15f,%.15f,%.15f]   Magnitude = %.15f\n",X(acc),Y(acc),Z(acc),magnitude);

    return acc;
}

/*WARNING: This potential is EVEN SLOWER than the Freeman Potential! Do NOT use this for NBody!*/
static inline mwvector sech2ExponentialDiskAccel(const Disk* disk, mwvector pos, real r)
{
    //mw_printf("Calculating Acceleration\n");
    //mw_printf("[X,Y,Z] = [%.15f,%.15f,%.15f]\n",X(pos),Y(pos),Z(pos));
    mwvector acc;

    const real R = mw_sqrt(sqr(X(pos)) + sqr(Y(pos)));
    mwvector R_hat;
    X(R_hat) = X(pos)/R;
    Y(R_hat) = Y(pos)/R;
    Z(R_hat) = 0.0;

    mwvector Z_hat;
    X(Z_hat) = 0.0;
    Y(Z_hat) = 0.0;
    Z(Z_hat) = 1.0;

    const real Rd = disk->scaleLength;
    const real zd = disk->scaleHeight;
    const real M = disk->mass;
    const real z = Z(pos);

    const int n = 15;
    const real a = 0.0;
    const real b = 60.0/R;
    real integralR = 0.0;
    real integralZ = 0.0;
    const real weight[] = {0.236927,0.478629,0.568889,0.478629,0.236927};
    const real point[] = {-0.90618,-0.538469,0.0,0.538469,0.90618};
    const real h = (b-a)/(n*1.0);

    for (int k = 0; k < n; k++)     /*Five-point Gaussian Quadrature*/
    {
        real Rpiece = 0.0;
        real Zpiece = 0.0;
        real k_val = 0.0;
        for (int j = 0; j < 5; j++)
        {
            k_val = h*point[j]/2 + a+(k*1.0+0.5)*h;
            Rpiece = Rpiece + h*weight[j]*RSechIntegrand(k_val,R,Rd,z,zd)/2.0;
            Zpiece = Zpiece + h*weight[j]*ZSechIntegrand(k_val,R,Rd,z,zd)/2.0;
        }
        integralR  = integralR + Rpiece;
        integralZ  = integralZ + Zpiece;
    }

    mwvector R_comp = mw_mulvs(R_hat, -2.0*M/3.1415926535*integralR);
    mwvector Z_comp = mw_mulvs(Z_hat, -2.0*M/3.1415926535*integralZ);

    X(acc) = X(R_comp) + X(Z_comp);
    Y(acc) = Y(R_comp) + Y(Z_comp);
    Z(acc) = Z(R_comp) + Z(Z_comp);

    //real magnitude = mw_sqrt(sqr(X(acc))+sqr(Y(acc))+sqr(Z(acc)));

    //mw_printf("Acceleration[AX,AY,AZ] = [%.15f,%.15f,%.15f]   Magnitude = %.15f\n",X(acc),Y(acc),Z(acc),magnitude);

    return acc;
}

/*Halo potentials*/

static inline mwvector logHaloAccel(const Halo* halo, mwvector pos, real r)
{
    mwvector acc;

    const real tvsqr = -2.0 * sqr(halo->vhalo);
    const real qsqr  = sqr(halo->flattenZ);
    const real d     = halo->scaleLength;
    const real zsqr  = sqr(Z(pos));

    const real arst  = sqr(d) + sqr(X(pos)) + sqr(Y(pos));
    const real denom = (zsqr / qsqr) +  arst;

    X(acc) = tvsqr * X(pos) / denom;
    Y(acc) = tvsqr * Y(pos) / denom;
    Z(acc) = tvsqr * Z(pos) / ((qsqr * arst) + zsqr);

    return acc;
}

static inline mwvector nfwHaloAccel(const Halo* halo, mwvector pos, real r)
{
    const real a  = halo->scaleLength;
    const real ar = a + r;
//     const real c  = a * sqr(halo->vhalo) * (r - ar * mw_log((r + a) / a)) / (0.2162165954 * cube(r) * ar);
    /* this is done to agree with NEMO. IDK WHY. IDK where 0.2162165954 comes from */
    const real c  = a * sqr(a) * 237.209949228 * (r - ar * mw_log((r + a) / a)) / ( cube(r) * ar);

    return mw_mulvs(pos, c);
}

/* CHECKME: Seems to have precision related issues for a small number of cases for very small qy */
static inline mwvector triaxialHaloAccel(const Halo* h, mwvector pos, real r)
{
    mwvector acc;

    /* TODO: More things here can be cached */
    const real qzs      = sqr(h->flattenZ);
    const real rhalosqr = sqr(h->scaleLength);
    const real mvsqr    = -sqr(h->vhalo);

    const real xsqr = sqr(X(pos));
    const real ysqr = sqr(Y(pos));
    const real zsqr = sqr(Z(pos));

    const real arst  = rhalosqr + (h->c1 * xsqr) + (h->c3 * X(pos) * Y(pos)) + (h->c2 * ysqr);
    const real arst2 = (zsqr / qzs) + arst;

    X(acc) = mvsqr * (((2.0 * h->c1) * X(pos)) + (h->c3 * Y(pos)) ) / arst2;

    Y(acc) = mvsqr * (((2.0 * h->c2) * Y(pos)) + (h->c3 * X(pos)) ) / arst2;

    Z(acc) = (2.0 * mvsqr * Z(pos)) / ((qzs * arst) + zsqr);

    return acc;
}

static inline mwvector ASHaloAccel(const Halo* h, mwvector pos, real r)
{
    const real gam = h->gamma;
    const real lam = h->lambda;
    const real M = h->mass;
    const real a = h->scaleLength;
    const real scaleR = r/a;
    const real scaleL = lam/a;
    real c;

    if (r<lam)
    {    c = -(M/sqr(a))*mw_pow(scaleR,gam-2)/(1+mw_pow(scaleR,gam-1));
    }
    else
    {    c = -(M/sqr(r))*mw_pow(scaleL,gam)/(1+mw_pow(scaleL,gam-1));
    }

    return mw_mulvs(pos, c/r);
}

static inline mwvector WEHaloAccel(const Halo* h, mwvector pos, real r)
{
    const real a = h->scaleLength;
    const real M = h->mass;
    const real sum2 = sqr(a) + sqr(r);

    const real c = (-M/r)*(a + mw_sqrt(sum2))/(sum2 + a*mw_sqrt(sum2));

    return mw_mulvs(pos, c/r);

}

static inline mwvector NFWMHaloAccel(const Halo* h, mwvector pos, real r)
{
    const real a = h->scaleLength;
    const real M = h->mass;
    const real ar = a + r;

    const real c = (-M/sqr(r))*(mw_log(ar/a)/r - 1/ar);

    return mw_mulvs(pos, c);

}

static inline mwvector plummerHaloAccel(const Halo* h, mwvector pos, real r)
{
    const real tmp = mw_sqrt(sqr(h->scaleLength) + sqr(r));

    return mw_mulvs(pos, -h->mass / cube(tmp));
}

static inline mwvector hernquistHaloAccel(const Halo* h, mwvector pos, real r)
{
    const real tmp = h->scaleLength + r;

    return mw_mulvs(pos, -h->mass / (r * sqr(tmp)));
}

static inline mwvector ninkovicHaloAccel(const Halo* h, mwvector pos, real r)      /*Special case of Ninkovic Halo (l1=0,l2=3,l3=2) (Ninkovic 2017)*/
{
    const real rho0 = h->rho0;
    const real a = h->scaleLength;
    const real lambda = h->lambda;

    const real z = r/a;
    const real zl = lambda/a;
    const real f = 4.0*3.1415926535/3.0*rho0*cube(a);

    real mass_enc;

    if (r > lambda)
    {
        mass_enc = f*(mw_log(1.0+cube(zl)) - cube(zl)/(1+cube(zl)));
    }
    else
    {
        mass_enc = f*(mw_log(1.0+cube(z)) - cube(z)/(1+cube(zl)));
    }

    mwvector acc = mw_mulvs(pos, -mass_enc/cube(r));

    return acc;
}

mwvector nbExtAcceleration(const Potential* pot, mwvector pos)
{
    mwvector acc, acctmp;
    const real r = mw_absv(pos);
    /*Calculate the Disk Accelerations*/
    switch (pot->disk.type)
    {
        case FreemanDisk:
            acc = freemanDiskAccel(&pot->disk, pos, r);
            break;
        case MiyamotoNagaiDisk:
            acc = miyamotoNagaiDiskAccel(&pot->disk, pos, r);
            break;
        case DoubleExponentialDisk:
            acc = doubleExponentialDiskAccel(&pot->disk, pos, r);
            break;
        case Sech2ExponentialDisk:
            acc = sech2ExponentialDiskAccel(&pot->disk, pos, r);
            break;
        case NoDisk:
            X(acc) = 0.0;
            Y(acc) = 0.0;
            Z(acc) = 0.0;
            break;
        case InvalidDisk:
        default:
            mw_fail("Invalid primary disk type in external acceleration\n");
    }
    /*Calculate Second Disk Accelerations*/
    switch (pot->disk2.type)
    {
        case FreemanDisk:
            acctmp = freemanDiskAccel(&pot->disk2, pos, r);
            break;
        case MiyamotoNagaiDisk:
            acctmp = miyamotoNagaiDiskAccel(&pot->disk2, pos, r);
            break;
        case DoubleExponentialDisk:
            acctmp = doubleExponentialDiskAccel(&pot->disk2, pos, r);
            break;
        case Sech2ExponentialDisk:
            acctmp = sech2ExponentialDiskAccel(&pot->disk2, pos, r);
            break;
        case NoDisk:
            X(acctmp) = 0.0;
            Y(acctmp) = 0.0;
            Z(acctmp) = 0.0;
            break;
        case InvalidDisk:
        default:
            mw_fail("Invalid secondary disk type in external acceleration\n");
    }
    mw_incaddv(acc, acctmp);

    /*Calculate the Halo Accelerations*/
    switch (pot->halo.type)
    {
        case LogarithmicHalo:
            acctmp = logHaloAccel(&pot->halo, pos, r);
            break;
        case NFWHalo:
            acctmp = nfwHaloAccel(&pot->halo, pos, r);
            break;
        case TriaxialHalo:
            acctmp = triaxialHaloAccel(&pot->halo, pos, r);
            break;
        case CausticHalo:
            acctmp = causticHaloAccel(&pot->halo, pos, r);
            break;
        case AllenSantillanHalo:
            acctmp = ASHaloAccel(&pot->halo, pos, r);
            break;
        case WilkinsonEvansHalo:
            acctmp = WEHaloAccel(&pot->halo, pos, r);
        case NFWMassHalo:
            acctmp = NFWMHaloAccel(&pot->halo, pos, r);
            break;
        case PlummerHalo:
            acctmp = plummerHaloAccel(&pot->halo, pos, r);
            break;
        case HernquistHalo:
            acctmp = hernquistHaloAccel(&pot->halo, pos, r);
            break;
        case NinkovicHalo:
            acctmp = ninkovicHaloAccel(&pot->halo, pos, r);
            break;
        case NoHalo:
            X(acctmp) = 0.0;
            Y(acctmp) = 0.0;
            Z(acctmp) = 0.0;
            break;
        case InvalidHalo:
        default:
            mw_fail("Invalid halo type in external acceleration\n");
    }

    mw_incaddv(acc, acctmp);
    /*Calculate the Bulge Accelerations*/
    switch (pot->sphere[0].type)
    {
        case HernquistSpherical:
            acctmp = hernquistSphericalAccel(&pot->sphere[0], pos, r);
            break;
        case PlummerSpherical:
            acctmp = plummerSphericalAccel(&pot->sphere[0], pos, r);
            break;
        case NoSpherical:
            X(acctmp) = 0.0;
            Y(acctmp) = 0.0;
            Z(acctmp) = 0.0;
            break;
        case InvalidSpherical:
        default:
            mw_fail("Invalid bulge type in external acceleration\n");
    }

    mw_incaddv(acc, acctmp);
    if (!isfinite(mw_absv(acc)))
    {
        mw_printf("ERROR: UNNATURAL ACCELERATION CALCULATED!\n");
        mw_printf("[X,Y,Z] = [%.15f,%.15f,%.15f]\n",X(pos),Y(pos),Z(pos));
        mw_printf("Acceleration = %.15f \n",mw_absv(acc));
    }
    return acc;
}













