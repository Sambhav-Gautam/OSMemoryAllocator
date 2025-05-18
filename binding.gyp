{
  "targets": [
    {
      "target_name": "mems",
      "sources": ["src/mems.c", "src/mems_node.c"],
      "include_dirs": ["src"],
      "cflags": ["-Wall"],
      "conditions": [
        ["OS=='win'", {
          "defines": ["_HAS_EXCEPTIONS=0"],
          "msvs_settings": {
            "VCCLCompilerTool": {
              "ExceptionHandling": "0"
            }
          }
        }],
        ["OS=='mac'", {
          "xcode_settings": {
            "GCC_ENABLE_CPP_EXCEPTIONS": "NO"
          }
        }]
      ]
    }
  ]
}
