module.exports = {
  devServer: {
  public:'192.168.178.20:8080',
    disableHostCheck: true
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
