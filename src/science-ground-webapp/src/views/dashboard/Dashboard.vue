<template>
  <v-container id="dashboard">
    <analytics-modal />

    <div
      style="margin-left: auto; margin-right: auto"
      :class="!$vuetify.breakpoint.smAndDown ? 'mobileResize' : ''"
    >
      <v-container v-if="!finishLoading" class="d-flex justify-center mt-10">
        <v-progress-circular indeterminate color="primary" />
      </v-container>

      <v-layout row v-if="finishLoading">
        <v-flex class="d-flex flex-row" v-for="item in items" :key="item.ID">
          <terrarium class="mr-3 ml-3" v-bind:terrariumName="item.TerrariumAlias" v-bind:terrariumId="item.ID" v-bind:terrariumSensors="item.Sensors"/>
        </v-flex>
      </v-layout>
    </div>
  </v-container>
</template>

<script>
import Terrarium from "./components/Terrarium";
import AnalyticsModal from "./components/AnalyticsModal.vue";
import Vue from "vue";

export default {
  name: "ScienceGround2021",
  components: { terrarium: Terrarium, analyticsModal: AnalyticsModal, },
  data() {
    return {
      items: [],
      finishLoading: false,
    };
  },
  mounted() {
    let self = this;

    Vue.axios
      .get("/data/terrariums/get")
      .then((res) => {
        self.finishLoading = true;
        console.log(res);
        self.items = res.data.data;
      })
      .catch((err) => {
        console.log(err);
      });
  },
};
</script>

<style lang="sass" scoped>
.mobileResize
  width: 70%
</style>
