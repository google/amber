// Copyright 2019 The Amber Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package com.google.amber;

import static org.junit.Assert.assertEquals;

import android.content.Context;
import android.os.Bundle;

import androidx.test.platform.app.InstrumentationRegistry;

import org.junit.Test;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

// Instrumented test used for launching Amber.
public class AmberLauncher {
  @Test
  public void LaunchAmber() {
    Context app_context = InstrumentationRegistry.getInstrumentation().getTargetContext();
    assertEquals("com.google.amber", app_context.getPackageName());

    Bundle args = InstrumentationRegistry.getArguments();

    int arg_index = 1;
    List<String> args_list = new ArrayList<>();

    while (true) {
      String arg = args.getString("arg" + arg_index);
      if (arg == null) {
        break;
      }
      args_list.add(arg);
      ++arg_index;
    }

    File outputDir = app_context.getExternalCacheDir();

    // This will typically be: /sdcard/Android/data/com.google.amber/cache/amber_std{out,err}.txt
    String stdout_file = args.getString("stdout", new File(outputDir, "amber_stdout.txt").toString());
    String stderr_file = args.getString("stderr", new File(outputDir, "amber_stderr.txt").toString());

    int res = Amber.androidHelper(args_list.toArray(new String[0]), stdout_file, stderr_file);

    // If the process crashes during the above call or we call System.exit below, the output
    // from `adb shell am instrument ...` will be:
    //  com.google.amber.AmberLauncher:INSTRUMENTATION_RESULT: shortMsg=Process crashed.
    //  INSTRUMENTATION_CODE: 0
    if (res != 0) {
      System.exit(res);
    }
  }
}
