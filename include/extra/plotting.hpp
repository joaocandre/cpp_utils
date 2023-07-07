//------------------------------------------------------------------------------
/// @file       Header-only wrappers around *matplotlib-cpp* for simpler plotting operations
///
#ifndef CPPUTILS_INCLUDE_EXTRA_PLOTTING_HPP_
#define CPPUTILS_INCLUDE_EXTRA_PLOTTING_HPP_

#include <vector>
#include <string>
#include <initializer_list>
#include <map>
#include <utility>
#include "storage/matrix.hpp"
#include "matplotlibcpp.h"
#include "mplibrary/primitive.hpp"

namespace extra {

namespace plt = matplotlibcpp;

using LineStyle = std::map< std::string, std::string >;

template < typename yT, typename xT = yT >
void plot_impl(const std::vector< yT >& y, const std::vector< xT >& x, const std::vector< std::string >& labels = { }, LineStyle style = { }) {
    if (!labels.empty()) {
        style.emplace("label", labels[0]);
    }
    if (!x.empty()) {
        assert(x.size() == y.size());
        plt::plot(x, y, style);
        plt::xlim(x[0], x.back());
    } else {
        plt::plot(y, style);
        plt::xlim(0, static_cast< int >(y.size() - 1));
    }
}


template < typename yT, typename xT = yT >
void plot_impl(const std::matrix< yT >& ys, const std::vector< xT >& x, const std::vector< std::string >& labels = { }, LineStyle style = { }) {
    if (!labels.empty()) {
        assert(labels.size() >= ys.rows());
    }
    for (size_t r = 0; r < ys.rows(); r++) {
        if (labels.empty()) {
            plot_impl< double >(ys.row(r), x, labels, style);
        } else {
            plot_impl< double >(ys.row(r), x, { labels[r] }, style);
        }
    }
}


template < typename yT, typename xT = yT >
void plot_impl(const std::initializer_list< std::vector< yT > >& ys, const std::vector< xT >& x, const std::vector< std::string >& labels = { }, LineStyle style = { }) {
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


template < typename Y = std::initializer_list< std::vector< double > >, typename X = std::vector< double > >
long plot(long fig, const Y& y, const X& x = {}, const std::string& name = "", const std::vector< std::string >& labels = { }, const std::string& ylabel = "", const std::string& xlabel = "", bool show = true, LineStyle style = { }) {
    try {
        auto fig_ = plt::figure(fig);
        plt::tight_layout();
        if (!name.empty()) {
            plt::title(name);
        }
        plot_impl(y, x, labels, style);
        if (!labels.empty()) {
            plt::legend();
            plt::legend({{"loc", "upper right"}});
        }
        if (!xlabel.empty()) {
            plt::xlabel(xlabel);
        }
        if (!ylabel.empty()) {
            plt::ylabel(ylabel);
        }
        if (show) {
            plt::show();
        }
        // plt::figure_size(1280, 720); // not working, creates a new figure
        return fig_;
    } catch (const std::runtime_error& err) {
        // ...
        std::cout << err.what() << std::endl;
        return -1;
    }
}


template < typename Y = std::initializer_list< std::vector< double > >, typename X = std::vector< double > >
long plot(const Y& y, const X& x = {}, const std::string& name = "", const std::vector< std::string >& labels = { }, const std::string& ylabel = "", const std::string& xlabel = "", bool show = true, LineStyle style = { }) {
    return plot(-1, y, x, name, labels, ylabel, xlabel, show, style);
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

}  // namespace extra

#endif  // CPPUTILS_INCLUDE_EXTRA_UTILS_HPP_
