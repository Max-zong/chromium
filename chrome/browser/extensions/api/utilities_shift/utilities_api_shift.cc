// Copyright 2024 The Shift Authors

#include "chrome/browser/extensions/api/utilities_shift/utilities_api_shift.h"

#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/version_info/version_info.h"
#include "chrome/browser/first_run/first_run.h"
#include "extensions/browser/event_router.h"

namespace extensions {

namespace OnClipboardContentChanged = api::utilities::OnClipboardContentChanged;
namespace GetVersionResults = api::utilities::GetVersion::Results;
namespace IsFirstRunResults = api::utilities::IsFirstRun::Results;

////////////////////////////////////////////////////////////////////////////////
// UtilitiesAPI:
////////////////////////////////////////////////////////////////////////////////

static base::LazyInstance<
    BrowserContextKeyedAPIFactory<UtilitiesAPI>>::DestructorAtExit g_factory =
    LAZY_INSTANCE_INITIALIZER;

// static
BrowserContextKeyedAPIFactory<UtilitiesAPI>*
UtilitiesAPI::GetFactoryInstance() {
  return g_factory.Pointer();
}

// static
UtilitiesAPI* UtilitiesAPI::Get(content::BrowserContext* context) {
  return BrowserContextKeyedAPIFactory<UtilitiesAPI>::Get(context);
}

UtilitiesAPI::UtilitiesAPI(content::BrowserContext* context)
    : browser_context_(context) {
  // Monitor when the following events are being listened to in order to know
  // when to start the task manager.
  EventRouter::Get(browser_context_)
      ->RegisterObserver(this, OnClipboardContentChanged::kEventName);
}

UtilitiesAPI::~UtilitiesAPI() {
  // This object has already been unregistered as an observer in Shutdown().
}

void UtilitiesAPI::Shutdown() {
  DCHECK(thread_checker_.CalledOnValidThread());
  EventRouter::Get(browser_context_)->UnregisterObserver(this);
}

void UtilitiesAPI::OnListenerAdded(const EventListenerInfo& details) {
  // The utilitiesEventRouter will observe the clipboard as long as there are
  // listeners for the utilities.OnClipboardContentChanged event.
  DCHECK(thread_checker_.CalledOnValidThread());
  utilities_event_router()->ListenerAdded(details.extension_id);
  LOG(INFO) << "Event Listener Added: " << details.extension_id
            << " Count: " << utilities_event_router()->GetListenerCount();
}

void UtilitiesAPI::OnListenerRemoved(const EventListenerInfo& details) {
  DCHECK(thread_checker_.CalledOnValidThread());
  // If a utilities.OnClipboardContentChanged event
  // listener is removed, then we let the
  // extension API know that it has one fewer listener.
  utilities_event_router()->ListenerRemoved(details.extension_id);
  LOG(INFO) << "Event Listener Removed: " << details.extension_id
            << " Count: " << utilities_event_router()->GetListenerCount();
}

void UtilitiesAPI::OnExtensionUnloaded(content::BrowserContext* browser_context,
                                       const Extension* extension,
                                       UnloadedExtensionReason reason) {
  DCHECK(thread_checker_.CalledOnValidThread());
  // If a utilities.OnClipboardContentChanged event
  // listener is removed, then we let the
  // extension API know that it has one fewer listener.
  utilities_event_router()->ExtensionUninstalled(extension->id());
  LOG(INFO) << "ExtensionUnloaded: " << extension->id()
            << " Count: " << utilities_event_router()->GetListenerCount();
}

UtilitiesEventRouter* UtilitiesAPI::utilities_event_router() {
  if (!utilities_event_router_.get()) {
    utilities_event_router_ =
        std::make_unique<UtilitiesEventRouter>(browser_context_);
  }
  return utilities_event_router_.get();
}

///////////////////////////////////////////////////////////////////////////////
// UtilitiesGetVersionFunction:
////////////////////////////////////////////////////////////////////////////////

ExtensionFunction::ResponseAction UtilitiesGetVersionFunction::Run() {
  std::string version = std::string(version_info::GetVersionNumber());
  return RespondNow(ArgumentList(GetVersionResults::Create(version)));
}

///////////////////////////////////////////////////////////////////////////////
// UtilitiesIsFirstRunFunction:
////////////////////////////////////////////////////////////////////////////////

ExtensionFunction::ResponseAction UtilitiesIsFirstRunFunction::Run() {
  bool is_first_run = first_run::IsChromeFirstRun();
  return RespondNow(ArgumentList(IsFirstRunResults::Create(is_first_run)));
}

}  // namespace extensions
