<template>
  <v-dialog
    v-model="isOpen"
    fullscreen
    hide-overlay
    transition="dialog-bottom-transition"
  >
    <v-card>
      <v-toolbar height="35" dark color="primary">
        <v-btn icon dark @click="isOpen = false">
          <v-icon>mdi-close</v-icon>
        </v-btn>
        <v-toolbar-title class="pa-0"
          >Andamento del terrario: {{ terrariumName }}</v-toolbar-title
        >
      </v-toolbar>

      <v-row no-gutters>
      
        <!-- GRAFICI -->
        <v-col :cols="!$vuetify.breakpoint.smAndDown ? '10' : '12'">
          <v-card elevation="2" class="mt-3 ml-4 mr-4">
            <v-container>
              
              <v-tabs v-model="tabs" center-active>
                <v-tab v-for="item in terrariumSensors" :key="item.ID">{{item.TypeOfMeasure}}</v-tab>      
              </v-tabs>

              <v-tabs-items v-model="tabs">
                <v-tab-item v-for="item in terrariumSensors" :key="item.ID" :eager="true">
                  <sensorchart v-bind:terrariumId="terrariumId" v-bind:sensorDatas="item"/>
                </v-tab-item>
              </v-tabs-items>

            </v-container>
          </v-card >
        </v-col>  <!-- GRAFICI -->
        <v-col :cols="!$vuetify.breakpoint.smAndDown ? '2' : '12'">
          <timefilsers />
        </v-col>
      </v-row>
    </v-card>
  </v-dialog>
</template>

<script>
import { EventBus } from "../../../main";
import VueApexCharts from "vue-apexcharts";
import TimeFilsers from "./TimeFilsers.vue";
import SensorChart from "./SensorChart.vue";

export default {
  name: "AnalyticsModal",
  components: {
    apexchart: VueApexCharts,
    timefilsers: TimeFilsers,
    sensorchart: SensorChart,
  },
  data() {
    return {
      isOpen: false,
      tabs: null,
      terrariumName: "",
      terrariumId: "",
      terrariumSensors:  []
    };
  },

  mounted() {
    let self = this;
    EventBus.$emit("updateChart");
    EventBus.$on("changeDiaalogState", (value) => {
      self.isOpen = value.visibility;
      self.terrariumName = value.terrariumName;
      self.terrariumId = value.terrariumId;
      self.terrariumSensors = value.sensorsData;
    });
  },

  methods: {},
};
</script>

