<template>

  <v-container id="dashboard" >

  <analytics-modal />

  <div style=" margin-left: auto; margin-right: auto; " :class="!$vuetify.breakpoint.smAndDown ? 'mobileResize' : ''">
  <v-layout row>
    <v-flex class="d-flex flex-row"  v-for="item in items" :key="item">
      <terrarium class="mr-3 ml-3"/>
    </v-flex>
  </v-layout>
  </div>
  
  </v-container>


</template>

<script>
  import Terrarium from './components/Terrarium';
  import AnalyticsModal from './components/AnalyticsModal.vue';
  import Vue from 'vue'

  export default {
    name: 'ScienceGround2021',
    components: { 'terrarium': Terrarium, 'analyticsModal': AnalyticsModal },
    data () {
     return{
        items:[1,1,1,1],
     }
    },
    mounted () {  
      let self = this;

      Vue.axios
      .get('/data/terrariums/get')
      .then((res) => {
        console.log(res);
      } ).catch((err) => {console.log(err)});
    },
  

  }
</script>

<style lang="sass" scoped>
    .mobileResize
        width:70%
    
</style>
