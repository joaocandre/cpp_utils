#ifndef _CPP_UTILS_EXTRA_PLOTTING_HPP_
#define _CPP_UTILS_EXTRA_PLOTTING_HPP_

#include <vector>
#include <string>
#include "storage/matrix.hpp"
#include "matplotlibcpp.h"
//#include "mplibrary/primitive.hpp"

namespace extra {

namespace plt = matplotlibcpp;

template < typename yT, typename xT = yT >
void plot_impl(const std::vector< yT >& y, const std::vector< xT >& x, const std::vector< std::string >& labels = { }) {
    if (!x.empty()) {
        assert(x.size() == y.size());
        plt::plot(x, y, {{ "label", labels.empty() ? "" : labels[0] }});
        plt::xlim(x[0], x.back());
    } else {
        plt::plot(y, {{ "label", labels.empty() ? "" : labels[0] }});
        plt::xlim(0, static_cast< int >(y.size() - 1));
    }
}

template < typename yT, typename xT = yT >
void plot_impl(const std::matrix< yT >& ys, const std::vector< xT >& x, const std::vector< std::string >& labels = { }) {
    if (!labels.empty()) {
        assert(labels.size() >= ys.rows());
    }
    for (size_t r = 0; r < ys.rows(); r++) {
        if (labels.empty()) {
            plot_impl< double >(ys.row(r), x);
        } else {
            plot_impl< double >(ys.row(r), x, { labels[r] });
        }
    }
}

template < typename yT, typename xT = yT >
void plot_impl(const std::initializer_list< std::vector< yT > >& ys, const std::vector< xT >& x, const std::vector< std::string >& labels = { }) {
    if (!labels.empty()) {
        assert(labels.size() >= ys.size());
    }
    size_t r = 0;
    for (const auto& y : ys) {
        if (labels.empty()) {
            plot_impl< double >(y, x);
        } else {
            plot_impl< double >(y, x, { labels[r++] });
        }
    }
}

template < typename Y, typename X = std::vector< double > >
void plot(const Y& y, const X& x = {}, const std::string& name = "", const std::vector< std::string >& labels = { }, const std::string& ylabel = "", const std::string& xlabel = "", bool show = true) {
    try {
        plt::figure();
        plt::title(name);
        plot_impl(y, x, labels);
        if (!labels.empty()) {
            plt::legend();
        }
        if (!xlabel.empty()) {
            plt::xlabel(xlabel);
        }
        if (!ylabel.empty()) {
            plt::ylabel(ylabel);
        }
        plt::tight_layout();
        if (show) {
            plt::show();
        }
    } catch (const std::runtime_error&) {
        // ...
    }
}

void plot(const motion::Primitive& primitive, const std::vector< double >& phase) {
    std::matrix< double > ys(primitive.size(), phase.size());
    for (size_t p = 0; p < phase.size(); p++) {
        for (size_t b = 0; b < primitive.size(); b++) {
            ys(b, p) = primitive[b]->value(phase[p]);
        }
    }
    plot(ys, phase, "Basis");
}


// template < typename T >
// inline void plot(const std::vector< T >& y, const std::string& name, bool show = true) {
//     matplotlibcpp::figure();
//     matplotlibcpp::plot(y);
//     matplotlibcpp::title(name);
//     matplotlibcpp::tight_layout();
//     matplotlibcpp::xlim(1, int(y.size()));
//     if (show) {
//         matplotlibcpp::show();
//     }
// }

// template < typename T >
// inline void plot(const std::vector< T >& x, const std::vector< T >& y, const std::string& name, bool show = true) {
//     matplotlibcpp::figure();
//     matplotlibcpp::plot(x, y);
//     matplotlibcpp::title(name);
//     matplotlibcpp::tight_layout();
//     matplotlibcpp::xlim(x[0], x.back());
//     if (show) {
//         matplotlibcpp::show();
//     }
// }


// template < typename T >
// inline void multiplot(const std::matrix< T >& ys, const std::string& name, bool show = true) {
//     matplotlibcpp::figure();
//     for (int r = 0; r < ys.rows(); r++) {
//         matplotlibcpp::plot(ys.row(r));
//     }
//     matplotlibcpp::title(name);
//     matplotlibcpp::tight_layout();
//     matplotlibcpp::xlim(1, int(ys.cols()));
//     if (show) {
//         matplotlibcpp::show();
//     }
// }


// template < typename T >
// inline void multiplot(const std::vector< T >& x, const std::matrix< T >& ys, const std::string& name, bool show = true) {
//     matplotlibcpp::figure();
//     for (int r = 0; r < ys.rows(); r++) {
//         matplotlibcpp::plot(x, ys.row(r));
//     }
//     matplotlibcpp::title(name);
//     matplotlibcpp::tight_layout();
//     matplotlibcpp::xlim(x[0], x.back());
//     if (show) {
//         matplotlibcpp::show();
//     }
// }

}  // namespace extra

#endif  // MPLEARN_INCLUDE_ETC_UTILS_HPP_
