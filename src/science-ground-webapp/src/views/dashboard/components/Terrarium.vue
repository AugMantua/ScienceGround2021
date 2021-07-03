<template>
  <v-hover v-slot="{ hover }">
    <v-card
      max-width="200"
      :elevation="hover ? 16 : 2"
      @click="changeModalVisibility()"
    >
      <div v-ripple="{ center: true }">
        <a style="color: inherit; text-decoration: cover">
          <v-img
            class="white--text align-end rounded-t immageSize"
            style=""
            src="../../../assets/18868_3PVyaYryNQAN.jpg"
          >
          </v-img>
          <v-card-title>{{ terrariumName }}</v-card-title>
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
  props: ["terrariumName", "terrariumId", "terrariumSensors"],
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
  max-width: 100px
  max-height: 100px
  object-fit: contain
  margin: 0 auto
</style>