#include <cmath>

// see /home/itsme/mac/myprj/geotracker/plain-filter.py

class LowPassFilter {
    std::vector<double> _h;
    double sinc(double x) {
        if (x==0)
            return 1;
        return std::sin(M_PI*x)/(M_PI*x);
    }

/*
a * b
               0*2
         0*1 + 1*2
   0*0 + 1*1 + 2*2
   1*0 + 2*1 + 3*2
   2*0 + 3*1 + 4*2
   3*0 + 4*1 + 5*2
   4*0 + 5*1 + 6*2
   5*0 + 6*1 + 7*2
   6*0 + 7*1 + 8*2
   7*0 + 8*1 + 9*2
   8*0 + 9*1
   9*0



   0*2
   0*1 + 1*2
   0*0 + 1*1 + 2*2
         1*0 + 2*1 + 3*2
               2*0 + 3*1 + 4*2
                     3*0 + 4*1 + 5*2
                           4*0 + 5*1 + 6*2
                                 5*0 + 6*1 + 7*2
                                       6*0 + 7*1 + 8*2
                                             7*0 + 8*1 + 9*2
                                                   8*0 + 9*1
                                                         9*0
*/
    auto convolve(const std::vector<double>& a, const std::vector<double>& b)
    {
        std::vector<double> res;

        for (auto j = b.begin()+b.size()/2 ; j > b.begin() ; --j)
        {
            double sum = 0;
            auto jj = j;
            for (auto i = a.begin() ; i < a.end() && jj < b.end() ; ++i, ++jj)
                sum += *i * *jj;
            res.push_back(sum);
        }
        for (auto i = a.begin() ; i < a.end() ; ++i)
        {
            double sum = 0;
            auto ii = i;
            for (auto j = b.begin() ; j < b.end() && ii < a.end() ; ++j, ++ii)
                sum += *ii * *j;
            res.push_back(sum);
        }

        return res;
    }
    LowPassFilter(double fc, double b)
    {
        int N = std::ceil(4/b);
        if (N%2 == 0)
            ++N;

        double sumh = 0;
        for (int n = 0 ; n < N ; ++n) {
            double h = sinc(2*fc*(n-(N-1)/2));
            double w = 0.42 - 0.5 * std::cos(2*M_PI*n/(N-1)) + 0.08*std::cos(4*M_PI*n/(N-1));

            _h.push_back( h*w );

            sumh += _h.back();
        }
        for (auto & hv : _h)
            hv /= sumh;
    }

    auto calc(const std::vector<double>& vec)
    {
        return convolve(vec, _h);
    }
};
