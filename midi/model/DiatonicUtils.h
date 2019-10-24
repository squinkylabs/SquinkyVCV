#pragma once

#include <vector>

/**
 * Pitch representation: c is zero.
 * Many functions have two forms:
 *  xxxC() which assumes C Major
 *  xxx(root, mode) which does not.
 */
class DiatonicUtils
{
public:

    /*****************************************************************
     * Utilities for C Major
     */
    static bool isNoteInC(int pitch);

    /**
     * Gets a vector of destination pitches after transpose.
     * @param transposeAmount is in semitones. must be >= 0 and <= 11
     */
    static std::vector<int> getTransposeInC(int transposeAmount);

    /*****************************************************************
     * Misc util
     */

    static void _dump(const char*, const std::vector<int>&);


    /*****************************************************************
     * Constants for the 12 pitches in a chromatic scale
     */
    static const int c = {0};
    static const int c_ = {1};
    static const int d = {2};
    static const int d_ = {3};
    static const int e = {4};
    static const int f = {5};
    static const int f_ = {6};
    static const int g = {7};
    static const int g_ = {8};
    static const int a = {9};
    static const int a_ = {10};
    static const int b = {11};


};