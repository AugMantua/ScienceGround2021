<template>
  <v-dialog
    v-model="isOpen"
    fullscreen
    hide-overlay
    transition="dialog-bottom-transition"
  >
    <v-card>
      <v-toolbar height="35" dark color="primary">
        <v-btn icon dark @click="close()">
          <v-icon>mdi-close</v-icon>
        </v-btn>
        <v-toolbar-title class="pa-0"
          >Andamento del terrario: {{ terrariumName }}</v-toolbar-title
        >
      </v-toolbar>

      <v-row no-gutters class="pa-1">
      
        <!-- GRAFICI -->
        <v-col :cols="!$vuetify.breakpoint.smAndDown ? '9' : '12'" :class="!$vuetify.breakpoint.smAndDown ? 'mt-9' : ''">
          <v-card elevation="0">
            <v-container style="border: thin solid #999999;" elevation="1">
              
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
        </v-col>  <!-- FILTRI -->
        <v-col :cols="!$vuetify.breakpoint.smAndDown ? '3' : '12'" :class="!$vuetify.breakpoint.smAndDown ? 'mt-9 d-flex align-start' : 'mt-2 d-flex align-start'"  >
          <timefilsers style="widht:100%"/>
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

    EventBus.$on("changeDialogState", (value) => {
      self.isOpen = value.visibility;
      self.terrariumName = value.terrariumName;
      self.terrariumId = value.terrariumId;
      self.terrariumSensors = value.sensorsData;
     
    });
  },

  methods: {
    close(){
      this.terrariumName = "";
      this.terrariumId = "";
      this.terrariumSensors = [];
      this.$forceUpdate();
      this.isOpen = false;
    }
  },
};
</script>

