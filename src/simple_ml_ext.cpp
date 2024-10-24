#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <cmath>
#include <iostream>

namespace py = pybind11;

namespace {

template<bool trans_a, bool trans_b>
struct MatMul {
   static void compute(const float *A, const float *B, float *C,
                      size_t m, size_t n, size_t k) {
       for (size_t i = 0; i < m; i++) {
           for (size_t j = 0; j < k; j++) {
               C[i * k + j] = 0;
               for (size_t l = 0; l < n; l++) {
                   float a_val = trans_a ? A[l * m + i] : A[i * n + l];
                   float b_val = trans_b ? B[j * n + l] : B[l * k + j];
                   C[i * k + j] += a_val * b_val;
               }
           }
       }
   }
};

}
void softmax_regression_epoch_cpp(const float *X, const unsigned char *y,
								  float *theta, size_t m, size_t n, size_t k,
								  float lr, size_t batch)
{
    /**
     * A C++ version of the softmax regression epoch code.  This should run a
     * single epoch over the data defined by X and y (and sizes m,n,k), and
     * modify theta in place.  Your function will probably want to allocate
     * (and then delete) some helper arrays to store the logits and gradients.
     *
     * Args:
     *     X (const float *): pointer to X data, of size m*n, stored in row
     *          major (C) format
     *     y (const unsigned char *): pointer to y data, of size m
     *     theta (float *): pointer to theta data, of size n*k, stored in row
     *          major (C) format
     *     m (size_t): number of examples
     *     n (size_t): input dimension
     *     k (size_t): number of classes
     *     lr (float): learning rate / SGD step size
     *     batch (int): SGD minibatch size
     *
     * Returns:
     *     (None)
     */

    /// BEGIN YOUR CODE
   auto Z = std::make_unique<float[]>(batch * k);
   auto exp_Z = std::make_unique<float[]>(batch * k);
   auto grad = std::make_unique<float[]>(n * k);
   auto softmax = std::make_unique<float[]>(batch * k);

   for (size_t i = 0; i < m; i += batch) {
       size_t current_batch = std::min(batch, m - i);

       auto X_batch = X + i * n;

       MatMul<false, false>::compute(X_batch, theta, Z.get(),
                                   current_batch, n, k);

       for(size_t b = 0; b < current_batch; b++) {
           float sum = 0;
           for (size_t j = 0; j < k; j++) {
               exp_Z[b * k + j] = std::exp(Z[b * k + j]);
               sum += exp_Z[b * k + j];
           }

           for (size_t j = 0; j < k; j++) {
               softmax[b * k + j] = exp_Z[b * k + j] / sum;
           }
       }

       for (size_t b = 0; b < current_batch; b++) {
           for (size_t j = 0; j < k; j++) {
               if (j == y[i + b]) {
                   softmax[b * k + j] -= 1.0f;
               }
           }
       }

       MatMul<true, false>::compute(X_batch, softmax.get(), grad.get(),
                                  n, current_batch, k);

       for (size_t l = 0; l < n; l++) {
           for (size_t j = 0; j < k; j++) {
               theta[l * k + j] -= lr * grad[l * k + j] / current_batch;
           }
       }
    }
    /// END YOUR CODE
}


/**
 * This is the pybind11 code that wraps the function above.  It's only role is
 * wrap the function above in a Python module, and you do not need to make any
 * edits to the code
 */
PYBIND11_MODULE(simple_ml_ext, m) {
    m.def("softmax_regression_epoch_cpp",
    	[](py::array_t<float, py::array::c_style> X,
           py::array_t<unsigned char, py::array::c_style> y,
           py::array_t<float, py::array::c_style> theta,
           float lr,
           int batch) {
        softmax_regression_epoch_cpp(
        	static_cast<const float*>(X.request().ptr),
            static_cast<const unsigned char*>(y.request().ptr),
            static_cast<float*>(theta.request().ptr),
            X.request().shape[0],
            X.request().shape[1],
            theta.request().shape[1],
            lr,
            batch
           );
    },
    py::arg("X"), py::arg("y"), py::arg("theta"),
    py::arg("lr"), py::arg("batch"));
}
