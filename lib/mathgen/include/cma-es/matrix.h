#ifndef SYMATHS_MATRIX_H
#define SYMATHS_MATRIX_H

#include <vector>

using Vector = std::vector<double>;
using Matrix = std::vector<Vector>;

inline Matrix make_matrix(size_t n, double val = 0.0) {
    return Matrix(n, Vector(n, val));
}

inline Matrix make_identity(size_t n) {
    Matrix I = make_matrix(n, 0.0);
    for (size_t i = 0; i < n; i++)
        I[i][i] = 1.0;
    return I;
}

inline double dot(const std::vector<double>& a, const Vector& b) {
    double s = 0.0;
    for (size_t i = 0; i < a.size(); i++)
        s += a[i] * b[i];
    return s;
}

inline double norm(const Vector& a) {
    return std::sqrt(dot(a, a));
}

inline void vector_scale(Vector& v, double s) {
    for (auto& x : v) x *= s;
}

inline Vector vector_add(const Vector& a, const Vector& b) {
    Vector out(a.size());
    for (size_t i = 0; i < a.size(); i++)
        out[i] = a[i] + b[i];
    return out;
}

inline Vector vector_sub(const Vector& a, const Vector& b) {
    Vector out(a.size());
    for (size_t i = 0; i < a.size(); i++)
        out[i] = a[i] - b[i];
    return out;
}

inline Vector mv_mul(const Matrix& A, const Vector& x) {
    size_t n = x.size();
    Vector out(n, 0.0);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            out[i] += A[i][j] * x[j];
    return out;
}

inline Matrix outer(const Vector& u, const Vector& v) {
    size_t n = v.size();
    Matrix out = make_matrix(n);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            out[i][j] = u[i] * v[j];
    return out;
}

inline Matrix matrix_add_scaled(const Matrix& A, const Matrix& B, double a, double b) {
    size_t n = A.size();
    Matrix out = make_matrix(n);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            out[i][j] = a * A[i][j] + b * B[i][j];
    return out;
}

inline void matrix_copy_into(const Matrix& A, Matrix& B) {
    for (size_t i = 0; i < A.size(); ++i)
        B[i] = A[i];
}

inline void decomp(const Matrix& C, Vector& values, Matrix& vecs) {
    size_t n = C.size();

    if (C.empty() || C[0].size() < 2)
        return;

    Matrix A = C;
    vecs = make_identity(n);

    size_t max_iter = 100 * n * n;
    double eps = 1e-12;

    for (size_t iter = 0; iter < max_iter; iter++) {
        size_t p = 0, q = 1;
        double max_val = std::abs(A[0][1]);
        for (size_t i = 0; i < n; i++)
            for (size_t j = i + 1; j < n; j++)
                if (std::abs(A[i][j]) > max_val) {
                    max_val = std::abs(A[i][j]);
                    p = i; q = j;
                }

        if (max_val < eps) break;

        //angle de la rotation de Givens
        double theta = 0.5 * std::atan2(2.0 * A[p][q], A[p][p] - A[q][q]);
        double c = std::cos(theta);
        double s = std::sin(theta);

        Matrix temp_A = A;
        for (size_t i = 0; i < n; i++) {
            if (i == p || i == q) continue;
            double aip = A[i][p], aiq = A[i][q];
            temp_A[i][p] = temp_A[p][i] =  c * aip + s * aiq;
            temp_A[i][q] = temp_A[q][i] = -s * aip + c * aiq;
        }
        temp_A[p][p] = c * c * A[p][p] + 2 * s * c * A[p][q] + s * s * A[q][q];
        temp_A[q][q] = s * s * A[p][p] - 2 * s * c * A[p][q] + c * c * A[q][q];
        temp_A[p][q] = temp_A[q][p] = 0.0;
        A = temp_A;

        for (size_t i = 0; i < n; i++) {
            double vip = vecs[i][p];
            double viq = vecs[i][q];
            vecs[i][p] =  c * vip + s * viq;
            vecs[i][q] = -s * vip + c * viq;
        }
    }

    values.resize(n);
    for (size_t i = 0; i < n; i++)
        values[i] = std::max(A[i][i], 1e-20);
}

inline Matrix make_BD(const Matrix& vecs, const Vector& values) {
    size_t n = values.size();
    Matrix BD = make_matrix(n);
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < n; j++)
            BD[i][j] = vecs[i][j] * std::sqrt(values[j]);
    return BD;
}


#endif