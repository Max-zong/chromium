// Copyright 2024 The Shift Authors

#ifndef CHROME_BROWSER_EXTENSIONS_API_UTILITIES_SHIFT_UTILITIES_API_SHIFT_H__
#define CHROME_BROWSER_EXTENSIONS_API_UTILITIES_SHIFT_UTILITIES_API_SHIFT_H__

#include "base/memory/raw_ptr.h"
#include "base/scoped_observation.h"
#include "base/threading/thread_checker.h"
#include "chrome/browser/extensions/api/utilities_shift/utilities_event_router_shift.h"
#include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/event_router.h"
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_registry.h"
#include "extensions/browser/extension_registry_observer.h"


class UtilitiesAPITest;

namespace extensions {

// Handles calls made via the chrome.utilities API. There is a separate instance of
// this class for each profile, as requests are tracked by extension ID, but a
// regular and incognito profile will share the same instance.
class UtilitiesAPI : public ExtensionRegistryObserver,
                     public BrowserContextKeyedAPI,
                     public EventRouter::Observer {
 public:
  explicit UtilitiesAPI(content::BrowserContext* context);

  UtilitiesAPI(const UtilitiesAPI&) = delete;
  UtilitiesAPI& operator=(const UtilitiesAPI&) = delete;

  ~UtilitiesAPI() override;

  // BrowserContextKeyedAPI implementation.
  static BrowserContextKeyedAPIFactory<UtilitiesAPI>* GetFactoryInstance();

  // Convenience method to get the UtilitiesAPI for a profile.
  static UtilitiesAPI* Get(content::BrowserContext* context);

  // KeyedService:
  void Shutdown() override;

  // EventRouter::Observer:
  void OnListenerAdded(const EventListenerInfo& details) override;
  void OnListenerRemoved(const EventListenerInfo& details) override;

  // ExtensionRegistryObserver implementation.
  void OnExtensionUnloaded(content::BrowserContext* browser_context,
                           const Extension* extension,
                           UnloadedExtensionReason reason) override;

  UtilitiesEventRouter* utilities_event_router();

 private:
  friend class BrowserContextKeyedAPIFactory<UtilitiesAPI>;

  // BrowserContextKeyedAPI implementation.
  static const char* service_name() { return "UtilitiesAPI"; }

  const raw_ptr<content::BrowserContext> browser_context_;

  // Created lazily on first access.
  std::unique_ptr<UtilitiesEventRouter> utilities_event_router_;


  base::ThreadChecker thread_checker_;

  // Listen to extension unloaded notification.
  base::ScopedObservation<ExtensionRegistry, ExtensionRegistryObserver>
      extension_registry_observation_{this};
};


// Implementation of the utilities.getVersion API.
class UtilitiesGetVersionFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("utilities.getVersion", UTILITIES_GETVERSION)

 protected:
  ~UtilitiesGetVersionFunction() override = default;

  // ExtensionFunction:
  ResponseAction Run() override;
};

// Implementation of the utilities.isFirstRun API.
class UtilitiesIsFirstRunFunction : public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("utilities.isFirstRun", UTILITIES_ISFIRSTRUN)

 protected:
  ~UtilitiesIsFirstRunFunction() override = default;

  // ExtensionFunction:
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_UTILITIES_SHIFT_UTILITIES_API_SHIFT_H__
