// Copyright 2024 The Shift Authors

#include "chrome/browser/extensions/api/utilities_shift/utilities_event_router_shift.h"

#include "extensions/browser/event_router.h"
#include "ui/base/clipboard/clipboard_monitor.h"

namespace extensions {

namespace OnClipboardContentChanged = api::utilities::OnClipboardContentChanged;

UtilitiesEventRouter::UtilitiesEventRouter(content::BrowserContext* context)
    : browser_context_(context) {}

UtilitiesEventRouter::~UtilitiesEventRouter() {}

int UtilitiesEventRouter::GetListenerCount() {
  int listens_count = 0;
  for (const auto& pair : extension_to_listen_count_) {
    listens_count += pair.second;
  }
  return listens_count;
}

void UtilitiesEventRouter::ListenerAdded(const std::string& extension_id) {
  // Only add this as an observer if it's the first listener being added.
  if (extension_to_listen_count_.empty()) {
    ui::ClipboardMonitor::GetInstance()->AddObserver(this);
  }

  // Increment the listener count for the given extension.
  ++extension_to_listen_count_[extension_id];
}

void UtilitiesEventRouter::ListenerRemoved(const std::string& extension_id) {
  auto iter = extension_to_listen_count_.find(extension_id);
  if (iter != extension_to_listen_count_.end()) {
    if (--iter->second == 0) {
      // Remove the extension from the map if its count drops to zero.
      extension_to_listen_count_.erase(iter);

      // Check if there are no more listeners at all.
      if (extension_to_listen_count_.empty()) {
        ui::ClipboardMonitor::GetInstance()->RemoveObserver(this);
      }
    }
  }
}

void UtilitiesEventRouter::ExtensionUninstalled(
    const std::string& extension_id) {
  // This method now only needs to remove the extension from the map if it
  // exists.
  auto iter = extension_to_listen_count_.find(extension_id);
  if (iter != extension_to_listen_count_.end()) {
    extension_to_listen_count_.erase(iter);
  }

  // After removal, check if there are no listeners left and remove this as an
  // observer.
  if (extension_to_listen_count_.empty()) {
    ui::ClipboardMonitor::GetInstance()->RemoveObserver(this);
  }
}

void UtilitiesEventRouter::OnClipboardDataChanged() {
  EventRouter* router = EventRouter::Get(browser_context_);
  if (router &&
      router->HasEventListener(OnClipboardContentChanged::kEventName)) {
    std::unique_ptr<Event> event(
        new Event(events::ON_CLIPBOARD_CONTENT_CHANGED,
                  OnClipboardContentChanged::kEventName, base::Value::List()));
    router->BroadcastEvent(std::move(event));
  }
}

}  // namespace extensions
