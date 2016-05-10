#include "transform.h"
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

vector<float> getTransformation(const poivec &left, const poivec &right)
{
    vector<float> ret(9);
    int n = min(left.size(), right.size());
    gsl_matrix* a = (gsl_matrix_alloc(2 * n, 9));
    gsl_matrix* at = (gsl_matrix_alloc(9, 2 * n));
    gsl_matrix* b = (gsl_matrix_alloc(9, 9));
    gsl_matrix* c = (gsl_matrix_alloc(9, 9));
    for (int i = 0; i < n; i++) {
        const poi& l = left[i];
        const poi& r = right[i];
        double u[] {l.x, l.y, 1., 0, 0, 0,
                    -r.x * l.x, -r.x * l.y, -r.x};
        double v[] {0, 0, 0, l.x, l.y, 1.,
                    -r.y * l.x, -r.y * l.y, -r.y};
        for (int j = 0; j < 9; j++) {
            gsl_matrix_set(a, i * 2, j, u[j]);
            gsl_matrix_set(a, i * 2 + 1, j, v[j]);
        }
    }
    gsl_matrix_transpose_memcpy(at, a);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1., 
                   at, a, 0., b);
    gsl_vector* s = (gsl_vector_alloc(9));
    gsl_vector* tmp = (gsl_vector_alloc(9));
    gsl_linalg_SV_decomp(b, c, s, tmp);
    for (int i = 0; i < 9; i++) {
        ret[i] = float(gsl_matrix_get(b, i, 8));
    }
    gsl_matrix_free(a);
    gsl_matrix_free(at);
    gsl_matrix_free(b);
    gsl_matrix_free(c);
    gsl_vector_free(s);
    gsl_vector_free(tmp);
    return ret;
}

poi transformPOI(const vector<float> &h, const poi &p)
{
    poi r = p;
    float k = p.x * h[6] + p.y * h[7] + h[8];
    r.x = (p.x * h[0] + p.y * h[1] + h[2]) / k;
    r.y = (p.x * h[3] + p.y * h[4] + h[5]) / k;
    return r;
}


vector<float> getRansacTransform(const poivec &left, const poivec &right, 
                                 float inlierR, float threshold)
{
    vector<float> ret(9);
    int size = min(left.size(), right.size());
    uniform_int_distribution<int> uint_dist(0, size - 1);
    mt19937 rnd;
    int hs = 4;
    poivec l(hs), r(hs);
    int k = 0;
    while (true) {
        int a[hs] {uint_dist(rnd), uint_dist(rnd), 
                    uint_dist(rnd), uint_dist(rnd)};
        for (int i = 0; i < hs; i++) {
            l[i] = left[a[i]];
            r[i] = right[a[i]];
        }
        ret = getTransformation(l, r);
        int cnt = 0;
        for (int i = 0; i < size; i++) {
            poi p = transformPOI(ret, left[i]);
            float dx = p.x - right[i].x;
            float dy = p.y - right[i].y;
            if (dx * dx + dy * dy <= inlierR * inlierR)
                cnt++;
        }
        if (float(cnt) / size >= threshold)
            break;
        if (k == 1000) {
            inlierR *= 1.05;
            k = 0;
        }
    }
    
    return ret;
}
