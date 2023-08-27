#pragma once

#include "neko_types.h"
#include "neko_exception.h"

namespace neko {

  namespace js {

    enum WrappedType
    {
      Wrapped_Console = 0, //!< Static: JSConsole
      Wrapped_Math, //!< Static: JSMath
      Wrapped_Game,
      Wrapped_Vector2, //!< Dynamic: Vector2
      Wrapped_Vector3, //!< Dynamic: Vector3
      Wrapped_Quaternion, //!< Dynamic: Quaternion
      Wrapped_Mesh, //!< Dynamic: DynamicMesh
      Wrapped_Model, //!< Dynamic: Model
      Wrapped_Text, //!< Dynamic: Text
      Wrapped_Entity,
      Max_WrappedType
    };

    enum WrappedFieldIndex
    {
      WrapField_Type,
      WrapField_Pointer,
      Max_WrapField
    };

    // extern const map<WrappedType, const char* const> c_wrappedTypeNames;

  }

}