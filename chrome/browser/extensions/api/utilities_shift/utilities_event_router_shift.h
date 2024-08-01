// Copyright 2024 The Shift Authors

#ifndef CHROME_BROWSER_EXTENSIONS_API_UTILITIES_SHIFT_UTILITIES_EVENT_ROUTE_SHIFT_H__
#define CHROME_BROWSER_EXTENSIONS_API_UTILITIES_SHIFT_UTILITIES_EVENT_ROUTE_SHIFT_H__

#include "base/memory/raw_ptr.h"
#include "chrome/common/extensions/api/utilities.h"
// #include "extensions/browser/browser_context_keyed_api_factory.h"
#include "extensions/browser/extension_function.h"
// #include "extensions/browser/extension_event_histogram_value.h"
#include "ui/base/clipboard/clipboard_observer.h"

namespace extensions {

// Observes the Clipboard and routes the notifications as events to the
// extension system.
class UtilitiesEventRouter : public ui::ClipboardObserver {
 public:
  explicit UtilitiesEventRouter(content::BrowserContext* context);

  UtilitiesEventRouter(const UtilitiesEventRouter&) = delete;
  UtilitiesEventRouter& operator=(const UtilitiesEventRouter&) = delete;

  ~UtilitiesEventRouter() override;

  // Called when an extension wants to listen to OnClipboardDataChanged events.
  void ListenerAdded(const std::string& extension_id);

  // Called when an extension with a listener exits or removes it.
  void ListenerRemoved(const std::string& extension_id);

  // Called when an extension uninstalled.
  void ExtensionUninstalled(const std::string& extension_id);

  // ui::ClipboardObserver implementation.
  void OnClipboardDataChanged() override;

  int GetListenerCount();

 private:
  raw_ptr<content::BrowserContext> browser_context_;

  // Count of listeners, so we avoid sending updates if no one is interested.
  std::map<const std::string, int> extension_to_listen_count_;
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_UTILITIES_SHIFT_UTILITIES_EVENT_ROUTE_SHIFT_H__
