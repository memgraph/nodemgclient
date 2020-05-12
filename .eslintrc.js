module.exports = {
  'env': {
    'amd': true,
    'browser': false,
    'es6': true,
    'jest/globals': true,
    'node': true,
  },
  'extends': [
    'google',
    'eslint:recommended',
    'plugin:node/recommended',
    'plugin:prettier/recommended',
  ],
  'globals': {
    'Atomics': 'readonly',
    'SharedArrayBuffer': 'readonly',
  },
  'parserOptions': {
    'ecmaVersion': 2020,
    'sourceType': 'module',
  },
  'plugins': ['prettier', 'jest'],
  'rules': {
    'prettier/prettier': [
      'error', {
        'singleQuote': true,
        'trailingComma': 'all',
        'printWidth': 80,
      }
    ],
    'max-len': ['error', {'code': 80, 'ignoreUrls': true}],
    'eqeqeq': 'warn',
    'new-cap': 'off',
    'require-jsdoc': 'off',
    'valid-jsdoc': 'off',
  },
  "overrides": [{
    "files": "**/*.spec.js",
    "rules": {
      "node/no-unpublished-require": 0,
    },
  }]
};
