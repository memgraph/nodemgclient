{
  'conditions': [
    ['OS=="win"', {
      'variables': {
       'mgclient_dir%': '<!(echo %MGCLIENT_DIR%)',
      },
    }]
  ],
  'targets': [
    {
      'target_name': 'nodemgclient',
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ],
      'cflags': [ '-fexceptions' ],
      'cflags_cc': [ '-fexceptions' ],
      'defines': [ 'NAPI_CPP_EXCEPTIONS=1' ],
      'sources': [ 'src/addon.cpp', 'src/client.cpp', 'src/glue.cpp' ],
      'include_dirs': [ "<!@(node -p \"require('node-addon-api').include\")", "build/mgclient/include" ],
      'conditions': [
        ['OS=="lin"', { 'libraries': [ "-lmgclient" ], 'dependencies': [ "<!@(node -p \"require('node-addon-api').gyp\")" ] }],
        ['OS=="win"', {
          "msbuild_settings": {
            "ClCompile": {
              "LanguageStandard": "stdcpp17"
            }
          },
          "include_dirs": [
            "<(mgclient_dir)\include",
            "<(mgclient_dir)\mgclient_cpp\include",
            "<(mgclient_dir)\\build\src",
          ],
          "libraries": [
            "-l<(mgclient_dir)\\build\src\Debug\mgclient.lib",
          ],
          "dependencies": [
            "<!(node -p \"require('node-addon-api').gyp\")",
          ],
          "copies": [
            {
              "destination": "<(PRODUCT_DIR)",
              "files": [
                "<(mgclient_dir)\\build\src\Debug\mgclient.dll",
              ],
            }
          ],
        }],
      ]
    }
  ]
}
