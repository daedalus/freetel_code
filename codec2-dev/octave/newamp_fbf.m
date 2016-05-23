% newamp_fbf.m
%
% Copyright David Rowe 2015
% This program is distributed under the terms of the GNU General Public License 
% Version 2
%
% Interactive Octave script to explore frame by frame operation of new amplitude
% modelling model.
%
% Usage:
%   Make sure codec2-dev is compiled with the -DDUMP option - see README for
%    instructions.
%   ~/codec2-dev/build_linux/src$ ./c2sim ../../raw/hts1a.raw --dump hts1a
%   $ cd ~/codec2-dev/octave
%   octave:14> newamp_fbf("../build_linux/src/hts1a",50)



function newamp_fbf(samname, f=10)
  newamp;
  more off;
  plot_spectrum = 1;
  dec_in_freq = 1;
  dec_in_time = 0;
  vq_en = 1;
  mask_en = 1;
  k = 10;

  % load up text files dumped from c2sim ---------------------------------------

  sn_name = strcat(samname,"_sn.txt");
  Sn = load(sn_name);
  sw_name = strcat(samname,"_sw.txt");
  Sw = load(sw_name);
  model_name = strcat(samname,"_model.txt");
  model = load(model_name);
  [frames tmp] = size(model);

  load vq;
  load d20_vq;

  prev_dk_ = zeros(1,2*k);

  % Keyboard loop --------------------------------------------------------------

  key = ' ';
  do 
    figure(1);
    clf;
    s = [ Sn(2*f-1,:) Sn(2*f,:) ];
    size(s);
    plot(s);
    axis([1 length(s) -20000 20000]);

    figure(2);
    clf;
    Wo = model(f,1);
    L = model(f,2);
    Am = model(f,3:(L+2));
    AmdB = 20*log10(Am);

    axis([1 4000 -20 80]);
    hold on;
    if plot_spectrum
      plot((1:L)*Wo*4000/pi, AmdB,";Am;r");
      plot((1:L)*Wo*4000/pi, AmdB,"r+");
    end

    [maskdB Am_freqs_kHz] = mask_model(AmdB, Wo, L);
    %a_non_masked_m = find(AmdB > maskdB);
    %maskdB = maskdB - 6;
    %maskdB(a_non_masked_m) = maskdB(a_non_masked_m) + 6;
    plot(Am_freqs_kHz*1000, maskdB, ';mask;g');

    if mask_en
      AmdB_ = AmdB = maskdB;
    else
      AmdB_ = AmdB = AmdB;
    end
    if dec_in_freq
      [tmp1 tmp2 D] = decimate_in_freq(AmdB, 0);
      [AmdB_ AmdB_cyclic D_cyclic dk D1] = decimate_in_freq(AmdB_, 1, k);
      if vq_en
        [AmdB_ AmdB_cyclic D_cyclic dk_ D1_ ind] = decimate_in_freq(AmdB, 1, k, vq);
        plot(Am_freqs_kHz*1000, AmdB_, ';vq;c');
      end

      % experimental differential in time
      % get mask from 20ms ago (two frames), VQ delta, put back together.

      diff_dk = dk - prev_dk_;
      [res tmp vq_ind] = mbest(d20_vq, diff_dk, 1);
      dk_d = prev_dk_ + tmp;
      prev_dk_ = dk_d;
      AmdB_d_ = params_to_mask(L, k, dk_d, D1);
      plot(Am_freqs_kHz*1000, AmdB_d_, ';vq diff;bk');
      
      %plot(Am_freqs_kHz*1000, AmdB_cyclic, ';mask cyclic;b');
      %AmdB_pf = AmdB_*1.5;
      %AmdB_pf += max(AmdB_) - max(AmdB_pf);
      %plot(Am_freqs_kHz*1000, AmdB_, ';ind vq;g');

      % option decode from indexes, used to test effect of bit errors on Wo

      if 0
        Wo_ = pi*169/4000;
        L_  = floor(pi/Wo_);
        if vq_en
          [dk_ D1_] = index_to_params(ind, vq);
        end
        maskdB_ = params_to_mask(L_, k, dk_, D1_);
        plot((1:L_)*Wo_*4000/pi, maskdB_, ';ind vq;b-+');
      end
    end

    axis([0 4000 00 80]);

    % Optional decimated parameters
    %   need to general model_ parameters either side
    
    if dec_in_time
      decimate = 4;
      model_  = set_up_model_(model, f, decimate, vq_en, vq);    
      [maskdB_dit Wo_ L_] = decimate_frame_rate(model_, decimate, f, frames, Am_freqs_kHz);
      plot((1:L_)*Wo_*4000/pi, maskdB_dit, ';mask dit;b');
    end

    hold off;

    if dec_in_freq
      % lets get a feel for the "spectrum" of the smoothed spectral envelope
      % this will give us a feel for how hard it is to code, ideally we would like
      % just a few coefficents to be non-zero

      figure(3)
      clf

      en = L/2+1;
      stem(D(2:en),'g')
      hold on;
      stem(D_cyclic(2:en),'b')
      hold off;

      % let plot the cumulative amount of energy in each DFT

      figure(4)
      clf
      plot(cumsum(D(2:en)/sum(D(2:en))),';cumsum;g');
      hold on;
      plot(cumsum(D_cyclic(2:en)/sum(D_cyclic(2:en))),';cumsum cyclic;b');
      hold off;
      axis([1 L 0 1])

      figure(5)
      clf
      stem(dk,'b');
      hold on;
      stem(dk_,'g');
      hold off;
    end

    % interactive menu ------------------------------------------

    printf("\rframe: %d  menu: n-next  b-back  q-quit  m-mask_en", f);
    fflush(stdout);
    key = kbhit();

    if (key == 'm')
      if mask_en
        mask_en = 0;
      else
        mask_en = 1; 
      end
    endif
    if (key == 'n')
      f = f + 1;
    endif
    if (key == 'b')
      f = f - 1;
    endif
  until (key == 'q')
  printf("\n");

endfunction

 
function model_ = set_up_model_(model, f, decimate, vq_en, vq)
    [frames nc] = size(model);
    model_ = zeros(frames, nc); 
    left_f = decimate*floor((f-1)/decimate)+1;
    right_f = left_f + decimate;

    model_(left_f,:) = set_up_maskdB_(model, left_f, vq_en, vq); 
    model_(left_f,:) = post_filter(model_(left_f,:));
    model_(right_f,:) = set_up_maskdB_(model, right_f, vq_en, vq); 
    model_(right_f,:) = post_filter(model_(right_f,:));

    model_(f,1) = model(f,1);  % Wo
    model_(f,2) = model(f,2);  % L
endfunction


function amodel_row = set_up_maskdB_(model, f, vq_en, vq) 
  [frames nc] = size(model);
  max_amp = 80;

  Wo = model(f,1);
  L = model(f,2);
  Am = model(f,3:(L+2));
  AmdB = 20*log10(Am);

  [maskdB Am_freqs_kHz] = mask_model(AmdB, Wo, L);

  if vq_en
    maskdB_ = decimate_in_freq(maskdB, 1, 10, vq);
  else
    maskdB_ = decimate_in_freq(maskdB, 1, 10);
  end

  maskdB_ = maskdB;
  
  amodel_row = zeros(1,nc);
  amodel_row(1) = Wo;
  amodel_row(2) = L;
  Am_ = zeros(1,max_amp);
  Am_ = 10 .^ (maskdB_(1:L)/20); 
  amodel_row(3:(L+2)) = Am_;
endfunction
