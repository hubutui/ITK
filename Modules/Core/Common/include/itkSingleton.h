/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkSingleton_h
#define itkSingleton_h

#include "itkMacro.h"
#include "itkSingletonMacro.h"
#include <map>
#include <functional>

/** \brief A function which does nothing
 *
 * This function is to be used to mark parameters as unused to suppress
 * compiler warning. It can be used when the parameter needs to be named
 * (i.e. itkNotUsed cannot be used) but is not always used. It ensures
 * that the parameter is not optimized out.
 */
template <typename T>
inline void
Unused(const T &){};

namespace itk
{
/** \class SingletonIndex
 * \brief Implementation detail.
 *
 * \ingroup ITKCommon
 */

class ITKCommon_EXPORT SingletonIndex
{
public:
  /** Standard class types. */
  using Self = SingletonIndex;

#ifndef ITK_LEGACY_REMOVE
  using SingletonData [[deprecated("The internal representation of the singleton data is private, and may not "
                                   "correspond with SingletonData anymore.")]] =
    std::map<std::string, std::tuple<void *, std::function<void(void *)>, std::function<void()>>>;
#endif

  // obtain a global registered in the singleton index under the
  // globalName, if unknown then nullptr will be returned.
  template <typename T>
  T *
  GetGlobalInstance(const char * globalName)
  {
    return static_cast<T *>(this->GetGlobalInstancePrivate(globalName));
  }


  // Returns true.
  //
  // It is assumed that the global will remain valid until the start
  // of globals being destroyed.
  template <typename T>
  bool
  SetGlobalInstance(const char *                globalName,
                    T *                         global,
                    std::function<void(void *)> itkNotUsed(func),
                    std::function<void()>       deleteFunc)
  {
    this->SetGlobalInstancePrivate(globalName, global, deleteFunc);
    return true;
  }

  /** Set/Get the pointer to GlobalSingleton.
   * Note that SetGlobalSingleton is not concurrent thread safe. */
  static Self *
  GetInstance();
  static void
  SetInstance(Self * instance);
  ~SingletonIndex();

private:
  // may return nullptr if string is not registered already
  //
  // access something like a std::map<std::string, void *> or
  // registered globals, it may be possible to restrict the held
  // classes to be derived from itk::LightObject, so dynamic cast can
  // work, and could use some type of Holder<T> class for intrinsic types
  void *
  GetGlobalInstancePrivate(const char * globalName);

  // global is added or set to the singleton index under globalName
  void
  SetGlobalInstancePrivate(const char * globalName, void * global, std::function<void()> deleteFunc);

  /** The static GlobalSingleton. This is initialized to nullptr as the first
   * stage of static initialization. It is then populated on the first call to
   * itk::Singleton::Modified() but it can be overridden with SetGlobalSingleton().
   * */
  std::map<std::string, std::tuple<void *, std::function<void()>>> m_GlobalObjects;
  static Self *                                                    m_Instance;
  //  static SingletonIndexPrivate * m_GlobalSingleton;
};


// A wrapper for a global variable registered in the singleton index.
template <typename T>
T *
Singleton(const char * globalName, std::function<void(void *)> itkNotUsed(func), std::function<void()> deleteFunc)
{
  static SingletonIndex * singletonIndex = SingletonIndex::GetInstance();
  Unused(singletonIndex);
  T * instance = SingletonIndex::GetInstance()->GetGlobalInstance<T>(globalName);
  if (instance == nullptr)
  {
    instance = new T;
    SingletonIndex::GetInstance()->SetGlobalInstance<T>(globalName, instance, {}, deleteFunc);
  }
  return instance;
}
} // end namespace itk

#endif
