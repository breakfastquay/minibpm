/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    MiniBPM
    A fixed-tempo BPM estimator for music audio
    Copyright 2012 Particular Programs Ltd.

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.

    Alternatively, if you have a valid commercial licence for MiniBPM
    obtained by agreement with the copyright holders, you may
    redistribute and/or modify it under the terms described in that
    licence.

    If you wish to distribute code using MiniBPM under terms other
    than those of the GNU General Public License, you must obtain a
    valid commercial licence before doing so.
*/

#ifndef _BQ_MINI_BPM_H_
#define _BQ_MINI_BPM_H_

#include <vector>

namespace breakfastquay {

/**
 * A fixed-tempo BPM estimator, self-contained and implemented in a
 * single C++ file.
 *
 * This may be used in two ways: either call estimateTempoOfSamples()
 * with a single in-memory buffer of all audio samples, or (if the
 * input data is streamed or cannot fit in memory) call process()
 * repeatedly with consecutive sample blocks of any size, followed by
 * estimateTempo() when all samples have been supplied.
 *
 * A single channel of audio only may be supplied (multi-channel is
 * not supported). To process multi-channel audio, average the
 * channels first.
 *
 * Method:
 *
 * - Take the audio as a sequence of overlapping time-domain
 *   frames. The frame size is chosen so that, following a Fourier
 *   transform, the frequency range up to about an octave above
 *   middle-C would take about half a dozen bins. This is a relatively
 *   short frame giving quite good time resolution.
 *
 * - For each frame, extract the low-frequency range into the
 *   frequency domain (up to a cutoff around 400-500 Hz) using a small
 *   filterbank. Also extract a single bin from a high frequency range
 *   (around 9K) for broadband noise, and calculate the overall RMS of
 *   the frame.  (The low-frequency feature is the main contributor to
 *   tempo estimation, the other two are used as fallbacks if there is
 *   not enough low-frequency information.)  Accumulate sequences of
 *   framewise spectral difference sums for the frequency domain
 *   information, and a sequence of the RMS values, across the
 *   duration of the audio.
 *
 * - When all audio has been processed, calculate an autocorrelation
 *   of each of the three features normalised to unity maximum, and
 *   calculate a weighted sum of the autocorrelations (discarding any
 *   phase difference between the three signals) with the
 *   low-frequency feature given the most weight.
 *
 * - Drag a comb filter across the subset of the summed
 *   autocorrelation sequence that corresponds to the plausible tempo
 *   range. Allocate to each autocorrelation lag a weighted sum of its
 *   value and the value of the element at the beats-per-bar multiple
 *   of its lag (because a lag is most likely to correspond to a beat
 *   if there is a good correlation for the bar length that the beat
 *   would imply -- in fact this is a better indicator than the
 *   correlation of the beat itself).
 *
 * - Apply a simplistic perceptual weighting filter to prefer tempi
 *   around 120-130bpm.
 *
 * - Find the peak of the resulting filtered autocorrelation and
 *   return its corresponding tempo.
 */
class MiniBPM
{
public:
    MiniBPM(float sampleRate);
    ~MiniBPM();

    /**
     * Set the range of valid tempi. The default is 55-190bpm.
     */
    void setBPMRange(double min, double max);
    void getBPMRange(double &min, double &max) const;

    /**
     * Set the number of beats per bar, if known. If unknown, leave at
     * the default (which is 4).
     */
    void setBeatsPerBar(int bpb);
    int getBeatsPerBar() const;

    /**
     * Return the estimated tempo in bpm of the music audio in the
     * given sequence of samples. nsamples contains the number of
     * samples. If the tempo cannot be estimated because the clip is
     * too short, return 0.
     *
     * You should use either this function, or a series of process()
     * calls followed by an estimateTempo() call. Do not call both
     * process() and estimateTempoOfSamples() on the same estimator
     * object.
     */
    double estimateTempoOfSamples(const float *samples, int nsamples);

    /**
     * Supply a single block of audio for processing. The block may be
     * of any length. Blocks are assumed to be contiguous
     * (i.e. without overlap).
     */
    void process(const float *samples, int nsamples);

    /**
     * Return the estimated tempo in bpm of the music audio in the
     * sequence of samples previously supplied through a series of
     * calls to process(). If the tempo cannot be estimated because
     * the clip is too short, return 0.
     */
    double estimateTempo();

    /**
     * Return all candidate tempi for the last tempo estimation that
     * was carried out, in order of likelihood (best first). The value
     * returned from estimateTempo or estimateTempoOfSamples will
     * therefore be the first in the returned sequence.
     */
    std::vector<double> getTempoCandidates() const;

    /**
     * Prepare the object to carry out another tempo estimation on a
     * new audio clip. You can either call this between uses, or
     * simply destroy this object and construct a new one.
     */
    void reset();

private:
    class D;
    D *m_d;
};

}

#endif
