#include "pch.h"
#include "CppUnitTest.h"
#include "../../src/Game/engine/Math.h"
#include "../../src/Game/engine/Spatial.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Engine;

// Required for Assert::AreEqual
// needs ToString impl for custom types
namespace Microsoft
{
	namespace VisualStudio
	{
		namespace CppUnitTestFramework
		{
			// Vec3
			template <>
			static std::wstring ToString<Vec3f>(const Vec3f& vec)
			{
				std::string s = vec.toString();
				return std::wstring(s.begin(), s.end());
			}

			// Vec2
			template <>
			static std::wstring ToString<Vec2f>(const Vec2f& vec)
			{
				std::string s = vec.toString();
				return std::wstring(s.begin(), s.end());
			}

			// TODO: Mat4x4
		}
	}
}

namespace UnitTests
{
	TEST_CLASS(TestVec2)
	{
		TEST_METHOD(Vec2Add)
		{
			const Vec2f a = Vec2f(1, 2);
			const Vec2f b = Vec2f(2, 3);
			Assert::AreEqual(Vec2f(3, 5), a + b);
		}

		TEST_METHOD(Vec2Sub)
		{
			Vec2f a = Vec2f(1, 2);
			Vec2f b = Vec2f(2, 3);
			Assert::AreEqual(Vec2f(-1, -1), a - b);
		}

		TEST_METHOD(Vec2Mult)
		{
			Vec2f a = Vec2f(1, 2);
			Vec2f b = Vec2f(2, 3);
			Assert::AreEqual(Vec2f(2, 6), a * b);
		}

		TEST_METHOD(Vec2MultFloat)
		{
			Vec2f a = Vec2f(1, 2);
			Assert::AreEqual(Vec2f(2, 4), a * 2);
		}

		TEST_METHOD(Vec2Negate)
		{
			Vec2f a = Vec2f(1, 2);
			Assert::AreEqual(Vec2f(-1, -2), -a);
		}

		TEST_METHOD(Vec2PlusEqual)
		{
			Vec2f a = Vec2f(1, 2);
			a += Vec2f(3, 3);
			Assert::AreEqual(Vec2f(4, 5), a);
		}

		TEST_METHOD(Vec2MinusEqual)
		{
			Vec2f a = Vec2f(1, 2);
			a -= Vec2f(3, 3);
			Assert::AreEqual(Vec2f(-2, -1), a);
		}

		TEST_METHOD(Vec2MultEqual)
		{
			Vec2f a = Vec2f(1, 2);
			a *= Vec2f(3, 3);
			Assert::AreEqual(Vec2f(3, 6), a);
		}

		TEST_METHOD(Vec2MultEqualFloat)
		{
			Vec2f a = Vec2f(1, 2);
			a *= 4;
			Assert::AreEqual(Vec2f(4, 8), a);
		}

		TEST_METHOD(Vec2DivEqual)
		{
			Vec2f a = Vec2f(1, 2);
			a /= Vec2f(3, 3);
			Assert::AreEqual(Vec2f(1.0f / 3.0f, 2.0f / 3.0f), a);
		}

		TEST_METHOD(Vec2DivEqualFloat)
		{
			Vec2f a = Vec2f(1, 2);
			a /= 4;
			Assert::AreEqual(Vec2f(1.0f / 4.0f, 2.0f / 4.0f), a);
		}

		TEST_METHOD(Vec2Length)
		{
			Vec2f a = Vec2f(1, 2);
			float len = a.length();
			float ans = Math::sqrt(5.0f);
			Assert::AreEqual(ans, len);
		}

		TEST_METHOD(Vec2LengthSquared)
		{
			Vec2f a = Vec2f(1, 2);
			float len = a.lengthSquared();
			Assert::AreEqual(5.0f, len);
		}

		TEST_METHOD(Vec2Equals)
		{
			Vec2f a = Vec2f(1, 2);
			Vec2f b = Vec2f(1, 2);
			Assert::AreEqual(true, a == b);
		}

		TEST_METHOD(Vec2NotEquals)
		{
			Vec2f a = Vec2f(1, 2);
			Vec2f b = Vec2f(1, 2);
			Assert::AreEqual(false, a != b);
		}
	};

	TEST_CLASS(TestVec3)
	{
	public:
		TEST_METHOD(Vec3Add)
		{
			const Vec3f a = Vec3f(1, 2, 3);
			const Vec3f b = Vec3f(2, 3, 4);
			Assert::AreEqual(Vec3f(3, 5, 7), a + b);
		}

		TEST_METHOD(Vec3Sub)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Vec3f b = Vec3f(2, 3, 4);
			Assert::AreEqual(Vec3f(-1, -1, -1), a - b);
		}

		TEST_METHOD(Vec3Mult)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Vec3f b = Vec3f(2, 3, 4);
			Assert::AreEqual(Vec3f(2, 6, 12), a * b);
		}

		TEST_METHOD(Vec3MultFloat)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Assert::AreEqual(Vec3f(2, 4, 6), a * 2);
		}

		TEST_METHOD(Vec3Negate)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Assert::AreEqual(Vec3f(-1, -2, -3), -a);
		}

		TEST_METHOD(Vec3PlusEqual)
		{
			Vec3f a = Vec3f(1, 2, 3);
			a += Vec3f(3, 3, 3);
			Assert::AreEqual(Vec3f(4, 5, 6), a);
		}

		TEST_METHOD(Vec3MinusEqual)
		{
			Vec3f a = Vec3f(1, 2, 3);
			a -= Vec3f(3, 3, 3);
			Assert::AreEqual(Vec3f(-2, -1, 0), a);
		}

		TEST_METHOD(Vec3MultEqual)
		{
			Vec3f a = Vec3f(1, 2, 3);
			a *= Vec3f(3, 3, 3);
			Assert::AreEqual(Vec3f(3, 6, 9), a);
		}

		TEST_METHOD(Vec3MultEqualFloat)
		{
			Vec3f a = Vec3f(1, 2, 3);
			a *= 4;
			Assert::AreEqual(Vec3f(4, 8, 12), a);
		}

		TEST_METHOD(Vec3DivEqual)
		{
			Vec3f a = Vec3f(1, 2, 3);
			a /= Vec3f(3, 3, 3);
			Assert::AreEqual(Vec3f(1.0f / 3.0f, 2.0f / 3.0f, 1.0f), a);
		}

		TEST_METHOD(Vec3DivEqualFloat)
		{
			Vec3f a = Vec3f(1, 2, 3);
			a /= 4;
			Assert::AreEqual(Vec3f(1.0f / 4.0f, 2.0f / 4.0f, 3.0f / 4.0f), a);
		}

		TEST_METHOD(Vec3Length)
		{
			Vec3f a = Vec3f(1, 2, 3);
			float len = a.length();
			float ans = Math::sqrt(14.0f);
			Assert::AreEqual(ans, len);
		}

		TEST_METHOD(Vec3LengthSquared)
		{
			Vec3f a = Vec3f(1, 2, 3);
			float len = a.lengthSquared();
			Assert::AreEqual(14.0f, len);
		}

		TEST_METHOD(Vec3Dot)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Vec3f b = Vec3f(4, 5, 6);
			Assert::AreEqual(32.0f, a.dot(b));
		}

		TEST_METHOD(Vec3Cross)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Vec3f b = Vec3f(3, 4, 5);
			Assert::AreEqual(Vec3f(-2, 4, -2), a.cross(b));
		}

		TEST_METHOD(Vec3Equals)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Vec3f b = Vec3f(1, 2, 3);
			Assert::AreEqual(true, a == b);
		}

		TEST_METHOD(Vec3NotEquals)
		{
			Vec3f a = Vec3f(1, 2, 3);
			Vec3f b = Vec3f(1, 2, 3);
			Assert::AreEqual(false, a != b);
		}
	};
}
