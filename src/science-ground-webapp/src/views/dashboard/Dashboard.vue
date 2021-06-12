<template>
  <v-container
    id="dashboard"
    style="height:100%"
    class="d-flex justify-center"

  >
  <div style=" margin-left: auto; margin-right: auto;">
  <v-layout row>
    <v-flex class="d-flex flex-row"  v-for="item in items" :key="item">
      <terrarium class="mr-3 ml-3" />
    </v-flex>
  </v-layout>
  </div>
  </v-container>
</template>

<script>
  import Terrarium from './components/Terrarium';
  import Vue from 'vue';

  export default {
    name: 'Dashboard',
    components: { 'terrarium': Terrarium },
    data () {
     return{
          items:[1,1,1,1]
     }
    },
    mounted () { 
    },

  }
</script>
