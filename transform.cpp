#include "transform.h"
#include "gimage.h"
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

vtransform getTransformation(const poivec &left, const poivec &right)
{
    vtransform ret(9);
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

poi transformPOI(const vtransform &h, const poi &p)
{
    poi r = p;
    float k = p.x * h[6] + p.y * h[7] + h[8];
    r.x = (p.x * h[0] + p.y * h[1] + h[2]) / k;
    r.y = (p.x * h[3] + p.y * h[4] + h[5]) / k;
    return r;
}

vtransform getRansacTransform(const poivec &left, const poivec &right, 
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
        if (k == 100) {
            inlierR *= 1.25f;
            k = 0;
        }
    }
    
    return ret;
}

struct hContext {
    vector<int> head;
    vector<int> next;
    vector<int> to;
    int cnt = 0;
    a4 a;
    int wsize, size, qx, qy, qs, qo;
    
    hContext(int wsize, int size, int qx, int qy, int qs, int qo, a4& a) {
        this->a = a;
        this->size = size;
        this->wsize = wsize;
        this->qx = qx;
        this->qy = qy;
        this->qs = qs;
        this->qo = qo;
        head.resize(wsize, -1);
        next.resize(size << 5);
        to.resize(size << 5);
        cnt = 0;
    }
    void add(int u, int v) {
        to[cnt] = v;
        next[cnt] = head[u];
        head[u] = cnt++;
    }
    inline void push(int& x, int& y, int& s, int& o, int& m) {
        int* p = &(a[x][y][s][o]);
        p[0]++;
        int id = (p - a.a);
        add(id, m);
    }
    inline void pusho(int& x, int& y, int& s, float& o, int& m) {
        if (s < 0 || s >= qs)
            return;
        int co = o;
        push(x, y, s, co, m);
        co = (co + 1) % qo;
        push(x, y, s, co, m);
    }
    inline void pushs(int& x, int& y, float& s, float& o, int& m) {
        if (y < 0 || y >= qy)
            return;
        int cs = s;
        pusho(x, y, cs, o, m);
        cs++;
        pusho(x, y, cs, o, m);
    }
    inline void pushy(int& x, float& y, float& s, float& o, int& m) {
        if (x < 0 || x >= qx)
            return;
        int cy = y;
        pushs(x, cy, s, o, m);
        cy++;
        pushs(x, cy, s, o, m);
    }
    inline void pushx(float& x, float& y, float& s, float& o, int& m) {
        int cx = x;
        pushy(cx, y, s, o, m);
        cx++;
        pushy(cx, y, s, o, m);
    }
};

vtransform getHoughTransform(const poivec &left, const poivec &right, 
                             int width, int height, float minScale, 
                             float maxScale, int qx, int qy, 
                             int qscale, int qorient)
{
    return getHough(left, right, width, height, minScale, maxScale, 
                    qx, qy, qscale, qorient).first;
}

pair<vtransform, int> getHough(const poivec &left, const poivec &right, 
                             int width, int height, float minScale, 
                             float maxScale, int qx, int qy, 
                             int qscale, int qorient) {
    int wsize = qx * qy * qscale * qorient;
    vector<int> va(qx * qy * qscale * qorient);
    a4 a(&va[0], qy, qscale, qorient);
    int size = min(left.size(), right.size());
    hContext context(wsize, size, qx, qy, qscale, qorient, a);
    
    for (int i = 0; i < size; i++) {
        const poi& l = left[i];
        const poi& r = right[i];
        float csin = sinf(r.orient);
        float ccos = cosf(r.orient);
        float x = l.bx * ccos - -l.by * csin;
        float y = -(l.bx * csin + -l.by * ccos);
        x *= r.scale;
        y *= r.scale;
        x += r.x;
        y += r.y;
        if (x < 0 || y < 0 || x > width - 1 || y > height - 1)
            continue;
        float scale = r.scale / l.scale;
        if (scale < minScale || scale > maxScale)
            continue;
        float orient = r.orient;
        x = x / width * qx;
        y = y / height * qx;
        scale = log2f(scale / minScale) / 
                log2f(maxScale / minScale + 1.f) * qscale;
        orient = orient / (M_PI * 2.) * qorient;
        
        context.pushx(x, y, scale, orient, i);
    }
    
    auto mme = max_element(begin(va), end(va));
    int id = mme - begin(va);
    
    poivec al, ar;
    al.reserve(size);
    ar.reserve(size);
    int cnt = 0;
    for (int i = context.head[id]; i != -1; i = context.next[i]) {
        int v = context.to[i];
        al.push_back(left[v]);
        ar.push_back(right[v]);
        cnt++;
    }
    if (cnt < 4) {
        vector<float> ret(9, 0);
        return make_pair(ret, cnt);
    }
    
    return make_pair(getTransformation(al, ar), cnt);
}
