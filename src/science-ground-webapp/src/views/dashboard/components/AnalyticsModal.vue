<template>

    <v-dialog
      v-model="isOpen"
      fullscreen
      hide-overlay
      transition="dialog-bottom-transition"
    >
      <v-card>
        <v-toolbar
          dark
          color="primary"
        >
          <v-btn
            icon
            dark
            @click="isOpen = false"
          >
            <v-icon>mdi-close</v-icon>
          </v-btn>
          <v-toolbar-title>Settings</v-toolbar-title>
        </v-toolbar>

      </v-card>
    </v-dialog>

</template>

<script>
  import { EventBus } from "../../../main"
  export default {
    
    name: 'AnalyticsModal',
    components: {
    
    },
     data () {
     return{
          isOpen:false
     }
    },
   
    mounted(){
        EventBus.$on("changeDiaalogState", (value)=>{this.isOpen = value;});
    }
    
  }
</script>
