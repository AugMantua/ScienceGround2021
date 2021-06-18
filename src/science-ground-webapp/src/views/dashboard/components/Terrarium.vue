<template>
<v-hover v-slot="{ hover }" >
  <v-card
    max-width="200"  :elevation="hover ? 16 : 2"  @click="changeModalVisibility()"
  > 
     
    <div  v-ripple="{ center: true }">
    <a style=" color: inherit;  text-decoration: cover;">
    <v-img 
      class="white--text align-end rounded-t immageSize"
      style=" "
      src="../../../assets/18868_3PVyaYryNQAN.jpg"
    >
     
    </v-img>
  <v-card-title>Terrario Pippo</v-card-title>
  </a>
  </div>
 
  </v-card>
   </v-hover>
</template>

<script>
  import { EventBus } from "../../../main"
  export default {
    name: 'Terrarim',
    components: { },
    
    data () {
     return{
     }
    },
    mounted () { 
    },
     methods:{
      changeModalVisibility(){
        EventBus.$emit("changeDiaalogState",true);
      },
    }
  }
</script>

<style lang="sass" scoped>
    .immageSize
        max-width: 100px
        max-height: 100px
        object-fit: contain
        margin: 0 auto
    
</style>