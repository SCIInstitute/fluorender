#ifndef LKG_CAMERA_H
#define LKG_CAMERA_H

#include <cmath>
#include <cstring>

class Vector3
{
public:
    float x, y, z;

    // Constructors
    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float xVal, float yVal, float zVal) : x(xVal), y(yVal), z(zVal) {}

    // Operator overloads
    Vector3 operator+(const Vector3& other) const
    {
        return Vector3(x + other.x, y + other.y, z + other.z);
    }

    Vector3 operator-(const Vector3& other) const
    {
        return Vector3(x - other.x, y - other.y, z - other.z);
    }

    Vector3 operator*(float scalar) const
    {
        return Vector3(x * scalar, y * scalar, z * scalar);
    }

    Vector3& operator+=(const Vector3& other)
    {
        x += other.x; y += other.y; z += other.z;
        return *this;
    }

    Vector3& operator-=(const Vector3& other)
    {
        x -= other.x; y -= other.y; z -= other.z;
        return *this;
    }

    Vector3& operator*=(float scalar)
    {
        x *= scalar; y *= scalar; z *= scalar;
        return *this;
    }

    // Vector operations
    float length() const
    {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector3 normalized() const
    {
        float len = length();
        if (len == 0.0f)
            return Vector3(0.0f, 0.0f, 0.0f);
        return Vector3(x / len, y / len, z / len);
    }

    static Vector3 cross(const Vector3& a, const Vector3& b)
    {
        return Vector3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static float dot(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};

class Matrix4
{
public:
    float m[16];

    // Constructors
    Matrix4()
    {
        std::memset(m, 0, 16 * sizeof(float));
    }

    Matrix4(const float* values)
    {
        std::memcpy(m, values, 16 * sizeof(float));
    }

    // Index operators
    float& operator[](int index)
    {
        return m[index];
    }

    const float& operator[](int index) const
    {
        return m[index];
    }

    // Multiplication
    Matrix4 operator*(const Matrix4& other) const
    {
        Matrix4 result;
        // Multiply two 4x4 matrices in column-major order
        for (int i = 0; i < 4; ++i) // Columns
        {
            for (int j = 0; j < 4; ++j) // Rows
            {
                result.m[i * 4 + j] = m[i * 4 + 0] * other.m[0 * 4 + j] +
                    m[i * 4 + 1] * other.m[1 * 4 + j] +
                    m[i * 4 + 2] * other.m[2 * 4 + j] +
                    m[i * 4 + 3] * other.m[3 * 4 + j];
            }
        }
        return result;
    }

    // Helper method to create an identity matrix
    static Matrix4 Identity()
    {
        Matrix4 result;
        result.m[0] = 1.0f;
        result.m[5] = 1.0f;
        result.m[10] = 1.0f;
        result.m[15] = 1.0f;
        return result;
    }
};

class LKGCamera
{
public:
    float size;        // Half-height of focal plane
    Vector3 center;    // Camera target (center)
    Vector3 up;        // Up vector
    float fov;         // Field of view in degrees
    float viewcone;    // Degrees from leftmost view to rightmost view, should be determined by display
    float aspectRatio; // Aspect ratio of the viewport
    float nearPlane;   // Near clipping plane
    float farPlane;    // Far clipping plane

    LKGCamera()
        : size(10.0f),
        center(0.0f, 0.0f, 0.0f),
        up(0.0f, 1.0f, 0.0f),
        fov(45.0f),
        viewcone(40.0f),
        aspectRatio(1.0f),
        nearPlane(0.1f),
        farPlane(100.0f)
    {
    }

    LKGCamera(float size, const Vector3& center, const Vector3& upVec,
        float fieldOfView, float viewcone, float aspect, float nearP, float farP)
        : size(size),
        center(center),
        up(upVec),
        fov(fieldOfView),
        viewcone(viewcone),
        aspectRatio(aspect),
        nearPlane(nearP),
        farPlane(farP)
    {
    }

    // Get the view matrix
    // Matrix4 getViewMatrix() const
    // {
    //     return computeViewMatrix(size, center, up);
    // }

    // Get the projection matrix
    Matrix4 getProjectionMatrix() const
    {
        return computeProjectionMatrix();
    }

    // Get the model matrix (e.g., for rotating an object)
    Matrix4 getModelMatrix(float angleX, float angleY) const
    {
        return computeModelMatrix(angleX, angleY);
    }

    // Get the camera's distance from center of focal plane, given FOV
    float getCameraDistance() const
    {
        return size / tan(fov * (3.1415926535f / 180.0f));
    }

    float getCameraOffset() const
    {
        return getCameraDistance() * tan(viewcone * (3.1415926535f / 180.0f));
    }

    // Compute view and projection matrices for hologram views
    void computeViewProjectionMatrices(float normalizedView, bool invert, float offset_mult, float focus, Matrix4& viewMatrix, Matrix4& projectionMatrix)
    {
        // Adjust camera position based on normalizedView and offset_mult
        float offset = -(normalizedView - 0.5f) * offset_mult * getCameraOffset();
        Vector3 adjustedPosition = center + Vector3(offset, 0.0f, 0.0f);

        // Adjust up vector if invert is true
        Vector3 adjustedUp = invert ? Vector3(up.x, -up.y, up.z) : up;

        // Compute the view matrix with the adjusted position and up vector
        viewMatrix = computeViewMatrix(size, center, adjustedUp, offset);

        // Compute the standard projection matrix
        projectionMatrix = computeProjectionMatrix();

        // Apply frustum shift to the projection matrix
        float viewPosition = normalizedView;
        float centerPosition = 0.5f;
        float distanceFromCenter = viewPosition - centerPosition;
        float frustumShift = distanceFromCenter * focus;

        // Modify the projection matrix to include frustum shift (column-major order)
        projectionMatrix[8] += (offset * 2.0f / (size * aspectRatio)) + frustumShift;
    }

private:
    // Helper method to compute the view matrix
    Matrix4 computeViewMatrix(float size, const Vector3& center, const Vector3& upVec, float offset) const
    {
        // Compute forward vector f = normalize(center - eye)
        // Vector3 f = (center - eye).normalized();
        Vector3 f = Vector3(0.0f, 0.0f, 1.0f); // todo: camera rotations

        // Compute up vector u = normalize(up)
        Vector3 u = upVec.normalized();

        // Compute s = normalize(cross(f, u))
        Vector3 s = Vector3::cross(f, u).normalized();

        // Recompute up vector u = cross(s, f)
        u = Vector3::cross(s, f);

        // Build the view matrix in column-major order
        Matrix4 matrix = Matrix4::Identity();

        matrix[0] = s.x;
        matrix[1] = u.x;
        matrix[2] = -f.x;
        matrix[3] = 0.0f;

        matrix[4] = s.y;
        matrix[5] = u.y;
        matrix[6] = -f.y;
        matrix[7] = 0.0f;

        matrix[8] = s.z;
        matrix[9] = u.z;
        matrix[10] = -f.z;
        matrix[11] = 0.0f;

        // Compute dot products for translation
        // float dot_s_eye = -Vector3::dot(s, eye);
        // float dot_u_eye = -Vector3::dot(u, eye);
        // float dot_f_eye = Vector3::dot(f, eye);

        // translation, todo: support rotation
        matrix[12] = offset;
        matrix[13] = 0.0f;
        matrix[14] = -getCameraDistance();
        matrix[15] = 1.0f;

        return matrix;
    }

    // Helper method to compute the projection matrix
    Matrix4 computeProjectionMatrix() const
    {
        float fov_rad = fov * (3.1415926535f / 180.0f);
        float f = 1.0f / std::tan(fov_rad / 2.0f);
        float aspect = aspectRatio;
        float n = nearPlane;
        float f_p = farPlane;

        Matrix4 matrix;

        matrix[0] = f / aspect;
        matrix[5] = f;
        matrix[10] = (f_p + n) / (n - f_p);
        matrix[11] = -1.0f;
        matrix[14] = (2 * f_p * n) / (n - f_p);

        return matrix;
    }

    // Helper method to compute the model matrix (for object transformations)
    Matrix4 computeModelMatrix(float angleX, float angleY) const
    {
        float cosX = std::cos(angleX);
        float sinX = std::sin(angleX);
        float cosY = std::cos(angleY);
        float sinY = std::sin(angleY);

        // Rotation around X-axis
        Matrix4 rotationX = Matrix4::Identity();
        rotationX[5] = cosX;
        rotationX[6] = -sinX;
        rotationX[9] = sinX;
        rotationX[10] = cosX;

        // Rotation around Y-axis
        Matrix4 rotationY = Matrix4::Identity();
        rotationY[0] = cosY;
        rotationY[2] = sinY;
        rotationY[8] = -sinY;
        rotationY[10] = cosY;

        // Combine rotations
        Matrix4 rotation = rotationY * rotationX;

        // Translation matrix (moving the object back by 3 units on Z-axis)
        Matrix4 translation = Matrix4::Identity();
        translation[14] = -3.0f;

        // Final model matrix
        Matrix4 modelMatrix = rotation * translation;
        return modelMatrix;
    }
};

#endif // LKG_CAMERA_H