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

        <v-col
          :cols="!$vuetify.breakpoint.smAndDown ? '9' : '12'"
          :class="!$vuetify.breakpoint.smAndDown ? 'mt-9' : ''"
        >
          <v-card elevation="0">
            <v-container style="border: thin solid #999999" elevation="1">
              <v-tabs v-model="tabs" center-active>
                <v-tab key="0" v-if="isOpen">{{ "Dati incrociati" }}</v-tab>
                <v-tab v-for="item in terrariumSensors" :key="item.ID">{{
                  item.TypeOfMeasure
                }}</v-tab>
              </v-tabs>

              <v-tabs-items v-model="tabs">
                <v-tab-item key="0">
                  <sensorchart
                    v-bind:terrariumId="terrariumId"
                    v-bind:sensorDatas="null"
                    v-if="isOpen"
                  />
                </v-tab-item>
                <v-tab-item
                  v-for="item in terrariumSensors"
                  :key="item.ID"
                  :eager="true"
                >
                  <sensorchart
                    v-bind:terrariumId="terrariumId"
                    v-bind:sensorDatas="item"
                  />
                </v-tab-item>
              </v-tabs-items>
            </v-container>
          </v-card>
        </v-col>
        <!-- FILTRI -->
        <v-col
          :cols="!$vuetify.breakpoint.smAndDown ? '3' : '12'"
          class="d-flex flex-column"
          :class="
            !$vuetify.breakpoint.smAndDown
              ? 'mt-9 d-flex align-start'
              : 'mt-2 d-flex align-start'
          "
        >
          <timefilters />
          <livefilters />
        </v-col>
      </v-row>
    </v-card>
  </v-dialog>
</template>

<script>
import { EventBus } from "../../../main";
import VueApexCharts from "vue-apexcharts";
import TimeFilters from "./TimeFilters.vue";
import LiveFilters from "./LiveFilters.vue";
import SensorChart from "./SensorChart.vue";
import Vue from "vue";
import moment from "moment";
import { TaskTimer } from "tasktimer";

export default {
  name: "AnalyticsModal",
  components: {
    apexchart: VueApexCharts,
    timefilters: TimeFilters,
    livefilters: LiveFilters,
    sensorchart: SensorChart,
  },
  data() {
    return {
      isOpen: false,
      tabs: null,
      terrariumName: "",
      terrariumId: "",
      terrariumSensors: [],

      liveTimer: null,
      liveModeEnabled: false,
    };
  },

  mounted() {
    let self = this;

    EventBus.$on("changeDialogState", (value) => {
      self.isOpen = value.visibility;
      self.terrariumName = value.terrariumName;
      self.terrariumId = value.terrariumId;
      self.terrariumSensors = value.sensorsData;

      let toFilter = new Date().toISOString().substr(0, 10);
      let fromFilter = moment(
        new Date().toISOString().substr(0, 10),
        "YYYY-MM-DD "
      )
        .subtract(3, "months")
        .format("YYYY-MM-DD");
      self.getSensorsMeasures(
        fromFilter.toString() + "00:00",
        toFilter.toString() + "23:59"
      );
    });

    EventBus.$on("filterUpdated", (value) => {
      if (value.onlyLast != undefined && !value.onlyLast) {
        self.liveModeEnabled = false;
        self.clearChart();
        this.liveTimer.stop();
      } else if (value.onlyLast != undefined && value.onlyLast) {
        self.liveModeEnabled = true;
        self.clearChart();
        self.startLiveChart();
      } else if (value.to != undefined && value.from != undefined) {
        self.getSensorsMeasures(value.from, value.to);
      }
    });

    this.liveTimer = new TaskTimer(1000);
    this.liveTimer.add({
      id: "live", // unique id of the task
      tickInterval: 5, // run every 5 ticks (5 x interval = 5000 ms)
      callback(task) {
        self.getSensorsMeasures("", "");
      },
    });
  },

  methods: {
    close() {
      this.terrariumName = "";
      this.terrariumId = "";
      this.terrariumSensors = [];
      this.isOpen = false;
      this.liveModeEnabled = false;
      this.liveTimer.stop();
    },
    startLiveChart() {
      let self = this;

      this.liveTimer.start();
    },
    clearChart() {
      EventBus.$emit("updateChart", {
        data: null,
      });
    },
    getSensorsMeasures(from, to) {
      let self = this;

      Vue.axios
        .get(
          "/data/measures/get?TerrariumID=" +
            self.terrariumId +
            "&From=" +
            from +
            "&To=" +
            to +
            "&LastUpdateOnly=" +
            self.liveModeEnabled
        )
        .then((res) => {
          let temp = [];
          if (res.data.data != null) {
            self.terrariumSensors.forEach((el) => {
              temp[el.ID] = [];
            });

            res.data.data.forEach((el) => {
              temp[el.SensorID].push({
                x: el.Timestamp,
                y: el.Value,
              });
            });

            let tempKeys = Object.keys(temp);
            tempKeys.forEach((el) => {
              EventBus.$emit("updateChart", {
                key: el,
                data: temp[el],
                liveMode: self.liveModeEnabled,
                sensors: self.terrariumSensors,
              });
            });
          }
        })
        .catch((err) => {
          self.loading = false;
        });
    },
  },
};
</script>

