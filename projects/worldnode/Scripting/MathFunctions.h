#pragma once
#include <Utils/DebugHandler.h>
#include "AngelBinder.h"
#include <entt.hpp>

#include <Math/Vector2.h>
#include <Math/Vector3.h>

//-----------------------
// AngelScript functions
//-----------------------

static void Vector2DefaultConstructor(Vector2* self)
{
    new (self) Vector2();
}

static void Vector2CopyConstructor(Vector2* self, const Vector2& other)
{
    new (self) Vector2(other);
}

static void Vector2CopyConstructorV3(Vector2* self, const Vector3& other)
{
    new (self) Vector2(other);
}

static void Vector2InitConstructor(Vector2* self, float x, float y)
{
    new (self) Vector2(x, y);
}

static void Vector3DefaultConstructor(Vector3* self)
{
    new (self) Vector3();
}

static void Vector3CopyConstructor(Vector3* self, const Vector3& other)
{
    new (self) Vector3(other);
}

static void Vector3CopyConstructorV2(Vector3* self, const Vector2& other)
{
    new (self) Vector3(other);
}

static void Vector3InitConstructor(Vector3* self, float x, float y, float z)
{
    new (self) Vector3(x, y, z);
}

void RegisterMathTypes(AB_NAMESPACE_QUALIFIER Engine* engine)
{
    int r;

    /// Vector2
    r = engine->asEngine()->RegisterObjectType("Vector2", sizeof(Vector2), asOBJ_VALUE | asOBJ_POD | asGetTypeTraits<Vector2>());
    assert(r >= 0);

    // variables
    r = engine->asEngine()->RegisterObjectProperty("Vector2", "float x", asOFFSET(Vector2, x));
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectProperty("Vector2", "float y", asOFFSET(Vector2, y));
    assert(r >= 0);

    // constructors
    r = engine->asEngine()->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector2DefaultConstructor), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in)", asFUNCTION(Vector2CopyConstructor), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(float, float y = 0)", asFUNCTION(Vector2InitConstructor), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);

    // operators
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 &opAddAssign(const Vector2 &in)", asMETHODPR(Vector2, operator+=,(const Vector2&), Vector2&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 &opSubAssign(const Vector2 &in)", asMETHODPR(Vector2, operator-=,(const Vector2&), Vector2&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 &opMulAssign(float)", asMETHODPR(Vector2, operator*=,(float), Vector2&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 &opDivAssign(float)", asMETHODPR(Vector2, operator/=,(float), Vector2&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "bool opEquals(const Vector2 &in) const", asMETHODPR(Vector2, operator==,(const Vector2&), bool), asCALL_THISCALL);
    assert(r >= 0);

    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opAdd(const Vector2 &in) const", asMETHODPR(Vector2, operator+,(const Vector2&), Vector2), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opSub(const Vector2 &in) const", asMETHODPR(Vector2, operator-,(const Vector2&), Vector2), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opMul(const Vector2 &in) const", asMETHODPR(Vector2, operator*,(const Vector2&), Vector2), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opDiv(const Vector2 &in) const", asMETHODPR(Vector2, operator/,(const Vector2&), Vector2), asCALL_THISCALL);
    assert(r >= 0);

    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opAdd(const float) const", asMETHODPR(Vector2, operator+,(const float), Vector2), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opSub(const float) const", asMETHODPR(Vector2, operator-,(const float), Vector2), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opMul(const float) const", asMETHODPR(Vector2, operator*,(const float), Vector2), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 opDiv(const float) const", asMETHODPR(Vector2, operator/,(const float), Vector2), asCALL_THISCALL);
    assert(r >= 0);

    // functions
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "float Length() const", asMETHOD(Vector2, Length), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "float SqrLength() const", asMETHOD(Vector2, SqrLength), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "float Dot(const Vector2 &in) const", asMETHOD(Vector2, Dot), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "Vector2 Normalize() const", asMETHOD(Vector2, Normalize), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector2", "string ToString() const", asMETHOD(Vector2, ToString), asCALL_THISCALL);
    assert(r >= 0);

    /// Vector3
    r = engine->asEngine()->RegisterObjectType("Vector3", sizeof(Vector3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CAK | asOBJ_APP_CLASS_ALLFLOATS /*asGetTypeTraits<Vector3>()*/);
    assert(r >= 0);

    // variables
    r = engine->asEngine()->RegisterObjectProperty("Vector3", "float x", asOFFSET(Vector3, x));
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectProperty("Vector3", "float y", asOFFSET(Vector3, y));
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectProperty("Vector3", "float z", asOFFSET(Vector3, z));
    assert(r >= 0);

    // constructors
    r = engine->asEngine()->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Vector3DefaultConstructor), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in)", asFUNCTION(Vector3CopyConstructor), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float y = 0, float z = 0)", asFUNCTION(Vector3InitConstructor), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);

    // operators
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 &opAddAssign(const Vector3 &in)", asMETHODPR(Vector3, operator+=,(const Vector3&), Vector3&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 &opSubAssign(const Vector3 &in)", asMETHODPR(Vector3, operator-=,(const Vector3&), Vector3&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 &opMulAssign(float)", asMETHODPR(Vector3, operator*=,(float), Vector3&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 &opDivAssign(float)", asMETHODPR(Vector3, operator/=,(float), Vector3&), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "bool opEquals(const Vector3 &in) const", asMETHODPR(Vector3, operator==,(const Vector3&), bool), asCALL_THISCALL);
    assert(r >= 0);

    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opAdd(const Vector3 &in) const", asMETHODPR(Vector3, operator+,(const Vector3&), Vector3), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opSub(const Vector3 &in) const", asMETHODPR(Vector3, operator-,(const Vector3&), Vector3), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opMul(const Vector3 &in) const", asMETHODPR(Vector3, operator*,(const Vector3&), Vector3), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opDiv(const Vector3 &in) const", asMETHODPR(Vector3, operator/,(const Vector3&), Vector3), asCALL_THISCALL);
    assert(r >= 0);

    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opAdd(const float) const", asMETHODPR(Vector3, operator+,(const float), Vector3), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opSub(const float) const", asMETHODPR(Vector3, operator-,(const float), Vector3), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opMul(const float) const", asMETHODPR(Vector3, operator*,(const float), Vector3), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 opDiv(const float) const", asMETHODPR(Vector3, operator/,(const float), Vector3), asCALL_THISCALL);
    assert(r >= 0);

    // functions
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "float Length() const", asMETHOD(Vector3, Length), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "float SqrLength() const", asMETHOD(Vector3, SqrLength), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "float Dot(const Vector3 &in) const", asMETHOD(Vector3, Dot), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "Vector3 Normalize() const", asMETHOD(Vector3, Normalize), asCALL_THISCALL);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectMethod("Vector3", "string ToString() const", asMETHOD(Vector3, ToString), asCALL_THISCALL);
    assert(r >= 0);

    // Cross type constructors, these need to be called after all types are defined
    r = engine->asEngine()->RegisterObjectBehaviour("Vector2", asBEHAVE_CONSTRUCT, "void f(const Vector3 &in)", asFUNCTION(Vector2CopyConstructorV3), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);
    r = engine->asEngine()->RegisterObjectBehaviour("Vector3", asBEHAVE_CONSTRUCT, "void f(const Vector2 &in)", asFUNCTION(Vector3CopyConstructorV2), asCALL_CDECL_OBJFIRST);
    assert(r >= 0);
}
