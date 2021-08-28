<template>
  <v-card outlined :class="!$vuetify.breakpoint.smAndDown ? 'ml-5' : ''" elevation="0" style="border: thin solid #999999;"   :width="!$vuetify.breakpoint.smAndDown ? '80%' :'100%'" >
    <v-card-title outlined class="ma-0 pa-0">Real Time</v-card-title>
       <v-switch color="red darken-3" label="Guarda in modalità RealTime" inset class="ml-3" @change="changeFilter()"  v-model="liveSwitchState"></v-switch>
  </v-card>
</template>

<script>
import Vue from "vue";
import { EventBus } from "../../../main";

export default {
  name: "LiveFilters",
  components: {},
  data() {
   return{
     liveSwitchState: false
   }
  },
  computed:{
  
  },

  watch:{
   
  },
  mounted() {

  },
  methods: {
    changeFilter() {
      let self = this;
      EventBus.$emit("updateChart", {
        onlyLast: self.liveSwitchState
      });
    }
  }
};
</script>
