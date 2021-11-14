{
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
        ['OS=="lin"', { 'libraries': [ "-lmgclient" ], 'dependencies': [ "<!@(node -p \"require('node-addon-api').gyp\")" ] }]
      ]
    }
  ]
}
