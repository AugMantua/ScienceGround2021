<template>
    <router-view />
</template>

<script>
  import Vue from 'vue'
  import axios from 'axios'
  import VueAxios from 'vue-axios'
  Vue.use(VueAxios, axios)

  const baseURL = 'http://lucacarre2400.ddns.net:8080';
  Vue.axios.defaults.baseURL = baseURL;
  Vue.axios.defaults.headers.common['Access-Control-Allow-Origin'] = '*';
  Vue.axios.defaults.headers.common['Content-Type'] = 'application/json';
  export default {
    name: 'App',
  }
</script>
<style lang="scss">

</style>