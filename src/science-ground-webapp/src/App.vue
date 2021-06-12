<template>
    <router-view />
</template>

<script>
  import Vue from 'vue'
  import axios from 'axios'
  import VueAxios from 'vue-axios'
  Vue.use(VueAxios, axios)
  export default {
    name: 'App',
  }
</script>
<style lang="scss">

</style>