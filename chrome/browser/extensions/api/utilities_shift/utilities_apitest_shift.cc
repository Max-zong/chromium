// Copyright 2024 The Shift Authors

#include "base/command_line.h"
#include "base/version_info/version_info.h"
#include "chrome/browser/extensions/api/utilities_shift/utilities_api_shift.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/extensions/extension_browsertest.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/test/browser_test.h"
#include "extensions/test/extension_test_message_listener.h"
#include "extensions/test/result_catcher.h"
#include "extensions/test/test_extension_dir.h"
#include "ui/base/clipboard/clipboard_buffer.h"
#include "ui/base/clipboard/scoped_clipboard_writer.h"

namespace extensions {

namespace {
constexpr char kManifest[] =
    R"({
          "name": "UtilitiesApiTest Extension",
          "version": "0.1",
          "manifest_version": 3,
          "permissions": ["utilities"],
          "background": {"service_worker": "background.js"}
        })";
}

class UtilitiesApiTest : public ExtensionApiTest {
 public:
  UtilitiesApiTest() {}

  UtilitiesApiTest(const UtilitiesApiTest&) = delete;
  UtilitiesApiTest& operator=(const UtilitiesApiTest&) = delete;

  ~UtilitiesApiTest() override {}

 protected:
  void SetUpOnMainThread() override {
    ExtensionBrowserTest::SetUpOnMainThread();
    base::RunLoop().RunUntilIdle();
  }

  int GetListenerCount() {
    return UtilitiesAPI::Get(profile())
        ->utilities_event_router()
        ->GetListenerCount();
  }
};

class UtilitiesApiForceFirstRunTest : public UtilitiesApiTest {
 protected:
  void SetUpCommandLine(base::CommandLine* command_line) override {
    command_line->AppendSwitch(switches::kForceFirstRun);
  }
};

// Test chrome.utilities.getVersion and permissions::utilities
IN_PROC_BROWSER_TEST_F(UtilitiesApiTest, GetVersion) {
  static constexpr char kJscriptFormat[] = R"(
    chrome.test.sendMessage("ready", async () => {
      chrome.utilities.getVersion(function(version) {
        chrome.test.assertEq(version, "%s");
        chrome.test.notifyPass();
      });
    });
  )";

  std::string background_js = base::StringPrintf(
      kJscriptFormat, version_info::GetVersionNumber().data());
  extensions::TestExtensionDir dir;
  dir.WriteManifest(kManifest);
  dir.WriteFile(FILE_PATH_LITERAL("background.js"), background_js.c_str());

  // Launch the test app.
  ExtensionTestMessageListener ready_listener("ready",
                                              ReplyBehavior::kWillReply);
  extensions::ResultCatcher result_catcher;
  LoadExtension(dir.UnpackedPath());

  // Run the test.
  EXPECT_TRUE(ready_listener.WaitUntilSatisfied());
  ready_listener.Reply("ok");
  EXPECT_TRUE(result_catcher.GetNextResult()) << result_catcher.message();
}

// Test chrome.utilities.IsFirstRun
IN_PROC_BROWSER_TEST_F(UtilitiesApiForceFirstRunTest, IsFirstRun) {
  static constexpr char kBackgroundJs[] = R"(
    chrome.test.sendMessage("ready", async () => {
      chrome.utilities.isFirstRun(function(firstRun) {
        chrome.test.assertEq(firstRun, true);
        chrome.test.notifyPass();
      });
    });
  )";

  extensions::TestExtensionDir dir;
  dir.WriteManifest(kManifest);
  dir.WriteFile(FILE_PATH_LITERAL("background.js"), kBackgroundJs);

  // Launch the test app.
  ExtensionTestMessageListener ready_listener("ready",
                                              ReplyBehavior::kWillReply);
  extensions::ResultCatcher result_catcher;
  LoadExtension(dir.UnpackedPath());

  // Run the test.
  EXPECT_TRUE(ready_listener.WaitUntilSatisfied());
  ready_listener.Reply("ok");
  EXPECT_TRUE(result_catcher.GetNextResult()) << result_catcher.message();
}

// Test chrome.utilities.OnClipboardContentChanged event.
IN_PROC_BROWSER_TEST_F(UtilitiesApiTest, OnClipboardContentChanged) {
  EXPECT_EQ(0, GetListenerCount());
  static constexpr char kBackgroundJs[] = R"(
    chrome.utilities.onClipboardContentChanged.addListener(function () {
     console.log("Received onClipboardContentChanged event.");
      chrome.test.notifyPass();
    });
    chrome.test.sendMessage("ready");
  )";

  extensions::TestExtensionDir dir;
  dir.WriteManifest(kManifest);
  dir.WriteFile(FILE_PATH_LITERAL("background.js"), kBackgroundJs);

  // Launch the test app.
  ExtensionTestMessageListener ready_listener("ready",
                                              ReplyBehavior::kWillReply);
  extensions::ResultCatcher result_catcher;
  const extensions::Extension* extension = LoadExtension(dir.UnpackedPath());

  // Run the test.
  EXPECT_TRUE(ready_listener.WaitUntilSatisfied());
  ready_listener.Reply("ok");
  EXPECT_GT(GetListenerCount(), 0);
  // Trigger Clipboard content changed.
  {
    ui::ScopedClipboardWriter writer(ui::ClipboardBuffer::kCopyPaste);
    writer.WriteText(u"");
  }
  {
    ui::ScopedClipboardWriter writer(ui::ClipboardBuffer::kCopyPaste);
    writer.WriteText(u"text");
  }

  EXPECT_TRUE(result_catcher.GetNextResult()) << result_catcher.message();
  EXPECT_GT(GetListenerCount(), 0);
  // Unload the extensions and make sure the listeners count is updated.
  UnloadExtension(extension->id());
  EXPECT_EQ(0, GetListenerCount());
}

}  // namespace extensions
