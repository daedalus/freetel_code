/*---------------------------------------------------------------------------*\
                                                                             
  FILE........: phase.c                                           
  AUTHOR......: David Rowe                                             
  DATE CREATED: 1/2/09                                                 
                                                                             
  Functions for modelling and synthesising phase.
                                                                             
\*---------------------------------------------------------------------------*/

/*
  Copyright (C) 2009 David Rowe

  All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2, as
  published by the Free Software Foundation.  This program is
  distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
  License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "defines.h"
#include "phase.h"
#include "four1.h"

#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define VTHRESH 4.0

/*---------------------------------------------------------------------------*\

  aks_to_H()

  Samples the complex LPC synthesis filter spectrum at the harmonic
  frequencies.

\*---------------------------------------------------------------------------*/

void aks_to_H(
	      MODEL *model,	/* model parameters */
	      float  aks[],	/* LPC's */
	      float  G,	        /* energy term */
	      COMP   H[],	/* complex LPC spectral samples */
	      int    order
)
{
  COMP  Pw[FFT_DEC];	/* power spectrum */
  int   i,m;		/* loop variables */
  int   am,bm;		/* limits of current band */
  float r;		/* no. rads/bin */
  float Em;		/* energy in band */
  float Am;		/* spectral amplitude sample */
  int   b;		/* centre bin of harmonic */
  float phi_;		/* phase of LPC spectra */

  r = TWO_PI/(FFT_DEC);

  /* Determine DFT of A(exp(jw)) ------------------------------------------*/

  for(i=0; i<FFT_DEC; i++) {
    Pw[i].real = 0.0;
    Pw[i].imag = 0.0;
  }

  for(i=0; i<=order; i++)
    Pw[i].real = aks[i];

  four1(&Pw[-1].imag,FFT_DEC,-1);

  /* Sample magnitude and phase at harmonics */

  for(m=1; m<=model->L; m++) {
    am = floor((m - 0.5)*model->Wo/r + 0.5);
    bm = floor((m + 0.5)*model->Wo/r + 0.5);
    b = floor(m*model->Wo/r + 0.5);

    Em = 0.0;
    for(i=am; i<bm; i++)
      Em += G/(Pw[i].real*Pw[i].real + Pw[i].imag*Pw[i].imag);
    Am = sqrt(fabs(Em/(bm-am)));

    phi_ = -atan2(Pw[b].imag,Pw[b].real);
    H[m].real = Am*cos(phi_);
    H[m].imag = Am*sin(phi_);
  }
}


/*---------------------------------------------------------------------------*\

   phase_synth_zero_order()

   Synthesises phases based on SNR and a rule based approach.  No phase 
   parameters are required apart from the SNR (which can be reduced to a
   1 bit V/UV decision per frame).

   The phase of each harmonic is modelled as the phase of a LPC
   synthesis filter excited by an impulse.  Unlike the first order
   model the position of the impulse is not transmitted, so we create
   an excitation pulse train using a rule based approach.  

   Consider a pulse train with a pulse starting time n=0, with pulses
   repeated at a rate of Wo, the fundamental frequency.  A pulse train
   in the time domain is equivalent to harmonics in the frequency
   domain.  We can make an excitation pulse train using a sum of
   sinsusoids:

     for(m=1; m<=L; m++)
       ex[n] = cos(m*Wo*n)

   Note: the Octave script ../octave/phase.m is an example of this if
   you would like to try making a pulse train.

   The phase of each excitation harmonic is:

     arg(E[m]) = mWo

   where E[m] are the complex excitation (freq domain) samples,
   arg(x), just returns the phase of a complex sample x.

   As we don't transmit the pulse position for this model, we need to
   synthesise it.  Now the excitation pulses occur at a rate of Wo.
   This means the phase of the first harmonic advances by N samples
   over a synthesis frame of N samples.  For example if Wo is pi/20
   (200 Hz), then over a 10ms frame (N=80 samples), the phase of the
   first harmonic would advance (pi/20)*80 = 4*pi or two complete
   cycles.

   We generate the excitation phase of the fundamental (first
   harmonic):

     arg[E[1]] = Wo*N;

   We then relate the phase of the m-th excitation harmonic to the
   phase of the fundamental as:

     arg(E[m]) = m*arg(E[1])

   This E[m] then gets passed through the LPC synthesis filter to
   determine the final harmonic phase.
     
   For a while there were prolems with low pitched males like hts1
   sounding "clicky".  The synthesied time domain waveform also looked
   clicky.  Many methods were tried to improve the sounds quality of
   low pitched males. Finally adding a small amount of jitter to each
   harmonic worked.

   The current result sounds very close to the original phases, with
   only 1 voicing bit per frame.  For example hts1a using original
   amplitudes and this phase model produces speech hard to distinguish
   from speech synthesise with the orginal phases.  The sound quality
   of this patrtiallyuantised codec (nb original amplitudes) is higher
   than g729, even though all the phase information has been
   discarded.

   NOTES:

     1/ This synthesis model is effectvely the same as simple LPC-10
     vocoders, and yet sounds much better.  Why?

     2/ I am pretty sure the Lincoln Lab sinusoidal coding guys (like xMBE
     also from MIT) first described this zero phase model, I need to look
     up the paper.

     3/ Note that this approach could cause some discontinuities in
     the phase at the edge of synthesis frames, as no attempt is made
     to make sure that the phase tracks are continuous (the excitation
     phases are continuous, but not the final phases after filtering
     by the LPC spectra).  Technically this is a bad thing.  However
     this may actually be a good thing, disturbing the phase tracks a
     bit.  More research needed, e.g. test a synthesis model that adds
     a small delta-W to make phase tracks line up for voiced
     harmonics.

     4/ Why does this sound so great with 1 V/UV decision?  Conventional
     wisdom says mixed voicing is required for high qaulity speech.

\*---------------------------------------------------------------------------*/

void phase_synth_zero_order(
    MODEL *model,
    float  aks[],
    float *ex_phase             /* excitation phase of fundamental */
)
{
  int   m;
  float new_phi;
  COMP  Ex[MAX_AMP];		/* excitation samples */
  COMP  A_[MAX_AMP];		/* synthesised harmonic samples */
  COMP  H[MAX_AMP];             /* LPC freq domain samples */
  float G;
  float jitter;

  G = 1.0;
  aks_to_H(model,aks,G,H,LPC_ORD);

  /* 
     Update excitation fundamental phase track, this sets the position
     of each pitch pulse during voiced speech.  After much experiment
     I found that using just this frame Wo improved quality for UV
     sounds compared to interpolating two frames Wo like this:
     
     ex_phase[0] += (*prev_Wo+mode->Wo)*N/2;
  */
  
  ex_phase[0] += (model->Wo)*N;
  ex_phase[0] -= TWO_PI*floor(ex_phase[0]/TWO_PI + 0.5);

  for(m=1; m<=model->L; m++) {

    /* generate excitation */

    if (model->voiced) {
	/* This method of adding jitter really helped remove the clicky
	   sound in low pitched makes like hts1a. This moves the onset
	   of each harmonic over at +/- 0.25 of a sample.
	*/
        jitter = 0.25*(1.0 - 2.0*rand()/RAND_MAX);
	Ex[m].real = cos(ex_phase[0]*m - jitter*model->Wo*m);
	Ex[m].imag = sin(ex_phase[0]*m - jitter*model->Wo*m);
    }
    else {

	/* When a few samples were tested I found that LPC filter
	   phase is not needed in the unvoiced case, but no harm in
	   keeping it.
        */
	float phi = TWO_PI*(float)rand()/RAND_MAX;
        Ex[m].real = cos(phi);
	Ex[m].imag = sin(phi);
    }

    /* filter using LPC filter */

    A_[m].real = H[m].real*Ex[m].real - H[m].imag*Ex[m].imag;
    A_[m].imag = H[m].imag*Ex[m].real + H[m].real*Ex[m].imag;

    /* modify sinusoidal phase */
   
    new_phi = atan2(A_[m].imag, A_[m].real+1E-12);
    model->phi[m] = new_phi;
  }

}
