<template>
  <v-hover v-slot="{ hover }">
    <v-card
      width="280"
      height="280"
      :elevation="hover ? 16 : 2"
      @click="changeModalVisibility()"
      style="border: thin solid #999999;"
    >
      <div v-ripple="{ center: true }">
        <a style="color: inherit; text-decoration: cover">
          <v-img
            class="white--text align-end rounded-t immageSize"
            style=""
            src="../../../assets/18868_3PVyaYryNQAN.png"
          >
          </v-img>
          <v-card-title class="d-inline-block text-truncate text-center">{{ terrariumName }}</v-card-title>
          <v-card-subtitle style="margin-top:1px">
            <p class="mb-1">{{"Tipo: " + TypeOfTerrarium}}</p>
            <p class="mb-1">{{"Ultima sincronizzazione: " + "11/11/2021 11:20"}}</p>
          </v-card-subtitle>
         
        </a>
      </div>
    </v-card>
  </v-hover>
</template>

<script>
import { EventBus } from "../../../main";
export default {
  name: "Terrarim",
  components: {},
  props: ["terrariumName", "terrariumId", "terrariumSensors", "TypeOfTerrarium"],
  data() {
    return {};
  },
  mounted() {},
  methods: {
    changeModalVisibility() {
      let self = this;
      EventBus.$emit("changeDiaalogState", {
        visibility: true,
        terrariumName: self.terrariumName,
        terrariumId: self.terrariumId,
        sensorsData: self.terrariumSensors
      });
    },
  },
};
</script>

<style lang="sass" scoped>
.immageSize
  max-width: 160px
  max-height: 200px
  object-fit: contain
  margin: 0 auto
</style>