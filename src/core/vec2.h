// Add to vec3.h or create vec2.h
class vec2 {
public:
    double e[2];
    
    vec2() : e{0, 0} {}
    vec2(double e0, double e1) : e{e0, e1} {}
    
    double u() const { return e[0]; }
    double v() const { return e[1]; }
    double x() const { return e[0]; }  // Alias for convenience
    double y() const { return e[1]; }  // Alias for convenience
    
    double operator[](int i) const { return e[i]; }
    double &operator[](int i) { return e[i]; }

    bool operator==(const vec2& other) const {
        return e[0] == other.e[0] && e[1] == other.e[1];
    }
};
    
using point2 = vec2;