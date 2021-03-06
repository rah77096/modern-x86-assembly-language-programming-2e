//------------------------------------------------
//               Ch14_04.cpp
//------------------------------------------------

#include "stdafx.h"
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <stdexcept>
#include "Ch14_04.h"
#include "AlignedMem.h"
#include "ImageMatrix.h"

using namespace std;

extern "C" uint64_t c_NumPixelsMax = 256 * 1024;

bool Avx512CalcImageStatsCpp(ImageStats& im_stats)
{
    uint64_t num_pixels = im_stats.m_NumPixels;
    const uint8_t* pb = im_stats.m_PixelBuffer;

    // Perform validation checks
    if ((num_pixels == 0) || (num_pixels > c_NumPixelsMax))
        return false;
    if (!AlignedMem::IsAligned(pb, 64))
        return false;

    // Calculate intermediate sums
    im_stats.m_PixelSum = 0;
    im_stats.m_PixelSumOfSquares = 0;
    im_stats.m_NumPixelsInRange = 0;
 
    for (size_t i = 0; i < num_pixels; i++)
    {
        uint32_t pval = pb[i];

        if (pval >= im_stats.m_PixelValMin && pval <= im_stats.m_PixelValMax)
        {
            im_stats.m_PixelSum += pval;
            im_stats.m_PixelSumOfSquares += pval * pval;
            im_stats.m_NumPixelsInRange++;
        }
    }

    // Calculate mean and standard deviation
    double temp0 = (double)im_stats.m_NumPixelsInRange * im_stats.m_PixelSumOfSquares;
    double temp1 = (double)im_stats.m_PixelSum * im_stats.m_PixelSum;
    double var_num = temp0 - temp1;
    double var_den = (double)im_stats.m_NumPixelsInRange * (im_stats.m_NumPixelsInRange - 1);
    double var = var_num / var_den;

    im_stats.m_PixelMean = (double)im_stats.m_PixelSum / im_stats.m_NumPixelsInRange;
    im_stats.m_PixelSd = sqrt(var);

    return true;
}

void Avx512CalcImageStats()
{
    const wchar_t* image_fn = L"..\\Ch14_Data\\TestImage4.bmp";

    ImageStats is1, is2;
    ImageMatrix im(image_fn);
    uint64_t num_pixels = im.GetNumPixels();
    uint8_t* pb = im.GetPixelBuffer<uint8_t>();

    is1.m_PixelBuffer = pb;
    is1.m_NumPixels = num_pixels;
    is1.m_PixelValMin = c_PixelValMin;
    is1.m_PixelValMax = c_PixelValMax;

    is2.m_PixelBuffer = pb;
    is2.m_NumPixels = num_pixels;
    is2.m_PixelValMin = c_PixelValMin;
    is2.m_PixelValMax = c_PixelValMax;

    const char nl = '\n';
    const char* s = " | ";
    const unsigned int w1 = 22;
    const unsigned int w2 = 12;

    cout << fixed << setprecision(6) << left;
    wcout << fixed << setprecision(6) << left;

    cout << "\nResults for Avx512CalcImageStats\n";
    wcout << setw(w1) << "image_fn:" << setw(w2) << image_fn << nl;
    cout << setw(w1) << "num_pixels:" << setw(w2) << num_pixels << nl;
    cout << setw(w1) << "c_PixelValMin:" << setw(w2) << c_PixelValMin << nl;
    cout << setw(w1) << "c_PixelValMax:" <<  setw(w2) << c_PixelValMax << nl;

    bool rc1 = Avx512CalcImageStatsCpp(is1);
    bool rc2 = Avx512CalcImageStats_(is2);

    if (!rc1 || !rc2)
    {
        cout << "Bad return code\n";
        cout << "  rc1 = " << rc1 << '\n';
        cout << "  rc2 = " << rc2 << '\n';
        return;
    }

    cout << nl;

    cout << setw(w1) << "m_NumPixelsInRange: ";
    cout << setw(w2) << is1.m_NumPixelsInRange << s;
    cout << setw(w2) << is2.m_NumPixelsInRange << nl;

    cout << setw(w1) << "m_PixelSum:";
    cout << setw(w2) << is1.m_PixelSum << s;
    cout << setw(w2) << is2.m_PixelSum << nl;

    cout << setw(w1) << "m_PixelSumOfSquares:";
    cout << setw(w2) << is1.m_PixelSumOfSquares << s;
    cout << setw(w2) << is2.m_PixelSumOfSquares << nl;

    cout << setw(w1) << "m_PixelMean:";
    cout << setw(w2) << is1.m_PixelMean << s;
    cout << setw(w2) << is2.m_PixelMean << nl;
    
    cout << setw(w1) << "m_PixelSd:";
    cout << setw(w2) << is1.m_PixelSd << s;
    cout << setw(w2) << is2.m_PixelSd << nl;
}

int main()
{
    try
    {
        Avx512CalcImageStats();
        Avx512CalcImageStats_BM();
    }

    catch (runtime_error& rte)
    {
        cout << "'runtime_error' exception has occurred - " << rte.what() << '\n';
    }

    catch (...)
    {
        cout << "Unexpected exception has occurred\n";
        cout << "File = " << __FILE__ << '\n';
    }

    return 0;
}
