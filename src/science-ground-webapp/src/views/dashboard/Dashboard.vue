<template>
  <v-container id="dashboard">
    <analytics-modal />

    <div
      style="margin-left: auto; margin-right: auto"
      :class="!$vuetify.breakpoint.smAndDown ? 'mobileResize' : ''"
    >
      <v-container v-if="!finishLoading" class="d-flex justify-center mt-7">
        <v-progress-circular indeterminate color="primary" />
      </v-container>

        <v-row id="container">
          <v-col
            v-for="item in items"
            :key="item.ID"
          > 
             <terrarium v-bind:terrarium="item"/>
          </v-col>
        </v-row>
 
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
      itemsPerRow: 4
    };
  },
  mounted() {
    let self = this;

    Vue.axios
      .get("/data/terrariums")
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
