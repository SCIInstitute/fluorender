/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2025 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef _SIZE_H_
#define _SIZE_H_

#include <iostream>

// Class for managing 2D size
class Size2D {
public:
	// Constructor
	inline Size2D() : width(0), height(0) {}
	inline Size2D(int width = 0, int height = 0) : width(width), height(height) {}
	inline Size2D(const Size2D& size) : width(size.width), height(size.height) {}
	inline Size2D& operator=(const Size2D& size) {
		width = size.width;
		height = size.height;
		return *this;
	}
	inline Size2D& operator=(int size) {
		width = size;
		height = size;
		return *this;
	}

	inline int& operator[](int i) {
		return i == 0 ? width : height;
	}

	inline const int& operator[](int i) const {
		return i == 0 ? width : height;
	}

	int operator==(const Size2D& size) const {
		return width == size.width && height == size.height;
	}

	int operator!=(const Size2D& size) const {
		return width != size.width || height != size.height;
	}

	inline Size2D operator+(const Size2D& size) const {
		return Size2D(width + size.width, height + size.height);
	}
	inline Size2D operator-(const Size2D& size) const {
		return Size2D(width - size.width, height - size.height);
	}
	inline Size2D operator*(const Size2D& size) const {
		return Size2D(width * size.width, height * size.height);
	}
	inline Size2D operator/(const Size2D& size) const {
		return Size2D(width / size.width, height / size.height);
	}

	inline Size2D operator+(int size) const {
		return Size2D(width + size, height + size);
	}
	inline Size2D operator-(int size) const {
		return Size2D(width - size, height - size);
	}
	inline Size2D operator*(int size) const {
		return Size2D(width * size, height * size);
	}
	inline Size2D operator/(int size) const {
		return Size2D(width / size, height / size);
	}

	inline Size2D operator+(double size) const {
		return Size2D(static_cast<int>(std::round(width + size)),
			static_cast<int>(std::round(height + size)));
	}
	inline Size2D operator-(double size) const {
		return Size2D(static_cast<int>(std::round(width - size)),
			static_cast<int>(std::round(height - size)));
	}
	inline Size2D operator*(double size) const {
		return Size2D(static_cast<int>(std::round(width * size)),
			static_cast<int>(std::round(height * size)));
	}
	inline Size2D operator/(double size) const {
		return Size2D(static_cast<int>(std::round(width / size)),
			static_cast<int>(std::round(height / size)));
	}

	inline Size2D& operator+=(const Size2D& size) {
		width += size.width;
		height += size.height;
		return *this;
	}
	inline Size2D& operator-=(const Size2D& size) {
		width -= size.width;
		height -= size.height;
		return *this;
	}
	inline Size2D& operator*=(const Size2D& size) {
		width *= size.width;
		height *= size.height;
		return *this;
	}
	inline Size2D& operator/=(const Size2D& size) {
		width /= size.width;
		height /= size.height;
		return *this;
	}

	inline Size2D& operator+=(int size) {
		width += size;
		height += size;
		return *this;
	}
	inline Size2D& operator-=(int size) {
		width -= size;
		height -= size;
		return *this;
	}
	inline Size2D& operator*=(int size) {
		width *= size;
		height *= size;
		return *this;
	}
	inline Size2D& operator/=(int size) {
		width /= size;
		height /= size;
		return *this;
	}

	inline Size2D& operator+=(double size) {
		int rounded = static_cast<int>(std::round(size));
		width += rounded;
		height += rounded;
		return *this;
	}
	inline Size2D& operator-=(double size) {
		int rounded = static_cast<int>(std::round(size));
		width -= rounded;
		height -= rounded;
		return *this;
	}
	inline Size2D& operator*=(double size) {
		width = static_cast<int>(std::round(width * size));
		height = static_cast<int>(std::round(height * size));
		return *this;
	}
	inline Size2D& operator/=(double size) {
		width = static_cast<int>(std::round(width / size));
		height = static_cast<int>(std::round(height / size));
		return *this;
	}

	inline int w() const { return width; }
	inline int h() const { return height; }
	inline void w(int w) { width = w; }
	inline void h(int h) { height = h; }

	inline bool isValid() const {
		return width > 0 && height > 0;
	}

	friend std::ostream& operator<<(std::ostream& os, const Size2D& size) {
		os << "[" << size.width << "," << size.height << "]";
		return os;
	}
	friend std::istream& operator>>(std::istream& is, Size2D& size) {
		char st;
		is >> st >> size.width >> st >> size.height >> st;
		return is;
	}
private:
	int width;
	int height;
};

// Class for managing 3D size
class Size3D {
public:
	// Constructor
	inline Size3D() : width(0), height(0), depth(0) {}
	inline Size3D(int width = 0, int height = 0, int depth = 0) : width(width), height(height), depth(depth) {}
	inline Size3D(const Size3D& size) : width(size.width), height(size.height), depth(size.depth) {}
	inline Size3D& operator=(const Size3D& size) {
		width = size.width;
		height = size.height;
		depth = size.depth;
		return *this;
	}
	inline Size3D& operator=(int size) {
		width = size;
		height = size;
		depth = size;
		return *this;
	}

	inline int& operator[](int i) {
		return i == 0 ? width : (i == 1 ? height : depth);
	}

	inline const int& operator[](int i) const {
		return i == 0 ? width : (i == 1 ? height : depth);
	}

	int operator==(const Size3D& size) const {
		return width == size.width && height == size.height && depth == size.depth;
	}

	int operator!=(const Size3D& size) const {
		return width != size.width || height != size.height || depth != size.depth;
	}

	inline Size3D operator+(const Size3D& size) const {
		return Size3D(width + size.width, height + size.height, depth + size.depth);
	}
	inline Size3D operator-(const Size3D& size) const {
		return Size3D(width - size.width, height - size.height, depth - size.depth);
	}
	inline Size3D operator*(const Size3D& size) const {
		return Size3D(width * size.width, height * size.height, depth * size.depth);
	}
	inline Size3D operator/(const Size3D& size) const {
		return Size3D(width / size.width, height / size.height, depth / size.depth);
	}

	inline Size3D operator+(int size) const {
		return Size3D(width + size, height + size, depth + size);
	}
	inline Size3D operator-(int size) const {
		return Size3D(width - size, height - size, depth - size);
	}
	inline Size3D operator*(int size) const {
		return Size3D(width * size, height * size, depth * size);
	}
	inline Size3D operator/(int size) const {
		return Size3D(width / size, height / size, depth / size);
	}

	inline Size3D operator+(double size) const {
		return Size3D(
			static_cast<int>(std::round(width + size)),
			static_cast<int>(std::round(height + size)),
			static_cast<int>(std::round(depth + size)));
	}
	inline Size3D operator-(double size) const {
		return Size3D(
			static_cast<int>(std::round(width - size)),
			static_cast<int>(std::round(height - size)),
			static_cast<int>(std::round(depth - size)));
	}
	inline Size3D operator*(double size) const {
		return Size3D(
			static_cast<int>(std::round(width * size)),
			static_cast<int>(std::round(height * size)),
			static_cast<int>(std::round(depth * size)));
	}
	inline Size3D operator/(double size) const {
		return Size3D(
			static_cast<int>(std::round(width / size)),
			static_cast<int>(std::round(height / size)),
			static_cast<int>(std::round(depth / size)));
	}

	inline Size3D& operator+=(const Size3D& size) {
		width += size.width;
		height += size.height;
		depth += size.depth;
		return *this;
	}
	inline Size3D& operator-=(const Size3D& size) {
		width -= size.width;
		height -= size.height;
		depth -= size.depth;
		return *this;
	}
	inline Size3D& operator*=(const Size3D& size) {
		width *= size.width;
		height *= size.height;
		depth *= size.depth;
		return *this;
	}
	inline Size3D& operator/=(const Size3D& size) {
		width /= size.width;
		height /= size.height;
		depth /= size.depth;
		return *this;
	}

	inline Size3D& operator+=(int size) {
		width += size;
		height += size;
		depth += size;
		return *this;
	}
	inline Size3D& operator-=(int size) {
		width -= size;
		height -= size;
		depth -= size;
		return *this;
	}
	inline Size3D& operator*=(int size) {
		width *= size;
		height *= size;
		depth *= size;
		return *this;
	}
	inline Size3D& operator/=(int size) {
		width /= size;
		height /= size;
		depth /= size;
		return *this;
	}

	inline Size3D& operator+=(double size) {
		int rounded = static_cast<int>(size);
		width += rounded;
		height += rounded;
		depth += rounded;
		return *this;
	}
	inline Size3D& operator-=(double size) {
		int rounded = static_cast<int>(size);
		width -= rounded;
		height -= rounded;
		depth -= rounded;
		return *this;
	}
	inline Size3D& operator*=(double size) {
		width = static_cast<int>(std::round(width * (size)));
		height = static_cast<int>(std::round(height * (size)));
		depth = static_cast<int>(std::round(depth * (size)));
		return *this;
	}
	inline Size3D& operator/=(double size) {
		width = static_cast<int>(std::round(width / (size)));
		height = static_cast<int>(std::round(height / (size)));;
		depth = static_cast<int>(std::round(depth / (size)));;
		return *this;
	}

	inline int w() const { return width; }
	inline int h() const { return height; }
	inline int d() const { return depth; }
	inline void w(int w) { width = w; }
	inline void h(int h) { height = h; }
	inline void d(int d) { depth = d; }

	inline bool isValid() const {
		return width > 0 && height > 0 && depth > 0;
	}

	friend std::ostream& operator<<(std::ostream& os, const Size3D& size) {
		os << "[" << size.width << "," << size.height << "," << size.depth << ']';
		return os;
	}
	friend std::istream& operator>>(std::istream& is, Size3D& size) {
		char st;
		is >> st >> size.width >> st >> size.height >> st >> size.depth >> st;
		return is;
	}

private:
	int width;
	int height;
	int depth;
};

#endif // _SIZE_H_