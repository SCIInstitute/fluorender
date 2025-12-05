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

#ifndef _BASE_RENDERER_H_
#define _BASE_RENDERER_H_
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <variant>

namespace flrd
{
	template <typename T>
	class RendererInput {
	public:
		using ValueType = T;
		using RefType = std::reference_wrapper<const T>;
		using PtrType = std::shared_ptr<T>;

		RendererInput(const T& value) : storage_(value) {}
		RendererInput(RefType ref) : storage_(ref) {}
		RendererInput(PtrType ptr) : storage_(ptr) {}

		const T& get() const {
			if (std::holds_alternative<ValueType>(storage_))
				return std::get<ValueType>(storage_);
			else if (std::holds_alternative<RefType>(storage_))
				return std::get<RefType>(storage_);
			else
				return *std::get<PtrType>(storage_);
		}

	private:
		std::variant<ValueType, RefType, PtrType> storage_;
	};

	class RendererSettings {
	public:
		virtual ~RendererSettings() = default;
	};

	class BaseRenderer {
	public:
		virtual ~BaseRenderer() = default;

		// Accepts a generic input wrapper
		template <typename T>
		void setInput(RendererInput<T> input) {
			setInputImpl(input);
		}

		virtual void setSettings(const std::shared_ptr<RendererSettings>& settings) = 0;
		virtual std::shared_ptr<RendererSettings> getSettings() = 0;
		virtual void render() = 0;
		virtual std::string type() const = 0;

	};
}

#endif//_BASE_RENDERER_H_