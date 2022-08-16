<template>
  <v-app>


    <router-view/>

  </v-app>
</template>

<script>
  import Vue from 'vue'
  import axios from 'axios'
  import VueAxios from 'vue-axios'
  Vue.use(VueAxios, axios)

  Vue.axios.defaults.baseURL = process.env.VUE_APP_API_BASE_URL;
  Vue.axios.defaults.headers.common['Access-Control-Allow-Origin'] = '*';
  Vue.axios.defaults.headers.common['Content-Type'] = 'application/json';
  export default {
    name: 'App',
  }
</script>
<style lang="scss">

</style>