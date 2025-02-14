// Copyright (c) Facebook, Inc. and its affiliates. (http://www.facebook.com)
#pragma once

#include "Python.h"

#include "Jit/deopt_patcher.h"
#include "Jit/ref.h"
#include "Jit/runtime.h"

#include <variant>

namespace jit {

class TypeDeoptPatcher : public DeoptPatcher {
 public:
  TypeDeoptPatcher(BorrowedRef<PyTypeObject> type);

  virtual bool shouldPatch(BorrowedRef<PyTypeObject> new_ty) const = 0;

 protected:
  void init() override;

  BorrowedRef<PyTypeObject> type_;
};

// Patch a DeoptPatchpoint when the given PyTypeObject no longer has a
// PyMemberDescr that fit the requires parameters for an optimized lookup.
class MemberDescrDeoptPatcher : public TypeDeoptPatcher {
 public:
  MemberDescrDeoptPatcher(
      BorrowedRef<PyTypeObject> type,
      BorrowedRef<PyUnicodeObject> member_name,
      int member_type,
      Py_ssize_t member_offset);

  void addReferences(CodeRuntime* code_rt) override;

  bool shouldPatch(BorrowedRef<PyTypeObject> new_ty) const override;

 private:
  BorrowedRef<PyUnicodeObject> member_name_;
  int member_type_;
  Py_ssize_t member_offset_;
};

class SplitDictDeoptPatcher : public TypeDeoptPatcher {
 public:
  SplitDictDeoptPatcher(
      BorrowedRef<PyTypeObject> type,
      BorrowedRef<PyUnicodeObject> attr_name,
      PyDictKeysObject* keys);

  void addReferences(CodeRuntime* code_rt) override;

  bool shouldPatch(BorrowedRef<PyTypeObject> new_ty) const override;

 private:
  BorrowedRef<PyUnicodeObject> attr_name_;

  // We don't need to grab a strong reference to keys_ like
  // MemberDescrDeoptPatcher does with member_name_ because calls to
  // PyTypeModified() happen before the old keys object is decrefed.
  PyDictKeysObject* keys_;
};

} // namespace jit
