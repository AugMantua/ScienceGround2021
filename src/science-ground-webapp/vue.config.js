module.exports = {
  devServer: {
    public: 'localhost:8081',
    disableHostCheck: true
  },

  configureWebpack: {
    devtool: 'source-map'
  },

  transpileDependencies: ['vuetify'],

  pluginOptions: {
    i18n: {
      locale: 'en',
      fallbackLocale: 'en',
      localeDir: 'locales',
      enableInSFC: false,
    },
  },
}
