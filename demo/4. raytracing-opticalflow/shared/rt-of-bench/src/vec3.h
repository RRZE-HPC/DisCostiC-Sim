#pragma once

struct Vec3 {
    double x, y, z;

    // constructors
    Vec3() {}

    Vec3(double xVal, double yVal, double zVal)
            : x(xVal), y(yVal), z(zVal) {}

    explicit Vec3(double scalar)
            : x(scalar), y(scalar), z(scalar) {}

    // basic operators
    Vec3 operator+(const Vec3 &other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3 &other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

    Vec3 operator*(double scalar) const {
        return {x * scalar, y * scalar, z * scalar};
    }

    Vec3 operator*(const Vec3 &other) const {
        return {x * other.x, y * other.y, z * other.z};
    }

    Vec3 operator/(double scalar) const {
        return {x / scalar, y / scalar, z / scalar};
    }

    // compound assignment operators
    Vec3 &operator+=(const Vec3 &other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Vec3 &operator-=(const Vec3 &other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3 &operator*=(double scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3 &operator/=(double scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    // dot product with another vector
    [[nodiscard]] double dot(const Vec3 &other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    // magnitude (length) of the vector
    [[nodiscard]] double magnitude() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // normalize the vector (make it a unit vector)
    [[nodiscard]] Vec3 normalize() const {
        return *this / magnitude();
    }

    [[nodiscard]] Vec3 rotate(double alpha, double beta, double gamma) const {
        const double rot[3][3] = {
                {
                        cos(alpha) * cos(beta),
                        cos(alpha) * sin(beta) * sin(gamma) - sin(alpha) * cos(gamma),
                        cos(alpha) * sin(beta) * cos(gamma) + sin(alpha) * sin(gamma)
                },
                {
                        sin(alpha) * cos(beta),
                        sin(alpha) * sin(beta) * sin(gamma) + cos(alpha) * cos(gamma),
                        sin(alpha) * sin(beta) * cos(gamma) - cos(alpha) * sin(gamma)
                },
                {
                        -sin(beta),
                        cos(beta) * sin(gamma),
                        cos(beta) * cos(gamma)
                }
        };

        return Vec3{
                rot[0][0] * x + rot[0][1] * y + rot[0][2] * z,
                rot[1][0] * x + rot[1][1] * y + rot[1][2] * z,
                rot[2][0] * x + rot[2][1] * y + rot[2][2] * z
        };
    }

    // print to std out
    void print() const {
        std::cout << "(" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

// free functions
double dot(const Vec3 &a, const Vec3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// scalar times vector
Vec3 operator*(double scalar, const Vec3 &vec) {
    return {scalar * vec.x, scalar * vec.y, scalar * vec.z};
}
