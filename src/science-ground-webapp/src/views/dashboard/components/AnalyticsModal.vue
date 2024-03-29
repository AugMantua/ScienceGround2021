<template>
  <v-dialog v-model="isOpen" fullscreen hide-overlay transition="dialog-bottom-transition">
    <v-card>
      <v-toolbar height="35" dark color="primary">
        <v-btn icon dark @click="close()">
          <v-icon>mdi-close</v-icon>
        </v-btn>
        <v-toolbar-title class="pa-0">Andamento del terrario: {{ terrariumName }}</v-toolbar-title>
      </v-toolbar>

      <v-row no-gutters class="pa-1">
        <!-- GRAFICI -->
        <v-col :cols="!$vuetify.breakpoint.smAndDown ? '9' : '12'" :class="!$vuetify.breakpoint.smAndDown ? 'mt-4' : ''">
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
                  <sensorchart v-bind:terrariumId="terrariumId" v-bind:sensorDatas="null" v-if="isOpen" />
                </v-tab-item>
                <v-tab-item v-for="item in terrariumSensors" :key="item.ID" :eager="true">
                  <sensorchart v-bind:terrariumId="terrariumId" v-bind:sensorDatas="item" />
                </v-tab-item>
              </v-tabs-items>
            </v-container>
          </v-card>
        </v-col>
        <!-- FILTRI -->
        <v-col :cols="!$vuetify.breakpoint.smAndDown ? '3' : '12'" class="d-flex flex-column" :class="!$vuetify.breakpoint.smAndDown
          ? 'mt-5 d-flex align-start'
          : 'mt-2 d-flex align-start'
          ">
          <timefilters v-if="isOpen" />
          <livefilters v-bind:liveStatus="liveModeEnabled" />

          <template>
            <v-card outlined :class="!$vuetify.breakpoint.smAndDown
              ? 'ml-5 mt-2 justify-center'
              : 'mt-2 justify-center'
              " elevation="0" style="border: thin solid #999999"
              :width="!$vuetify.breakpoint.smAndDown ? '80%' : '100%'">
              <v-card-title outlined class="ma-0 pa-0">Sessioni Dimostrative</v-card-title>
              <v-list dense class="mb-1 mt-2 pa-0 overflow-y-auto" style="max-height: 400px" rounded>
                <v-list-item style="width: 100%">
                  <v-list-item-group color="primary" v-model="selectedSession">
                    <v-list-item v-for="(item, i) in terrariumSession" :key="i">
                      <v-list-item-icon>
                        <v-icon>mdi-access-point</v-icon>
                      </v-list-item-icon>
                      <v-list-item-content>
                        <v-list-item-title class="font-weight-black">Da: {{ formatDate(item.TimestampStart) }}
                          {{ " " }} A:
                          {{ formatDate(item.TimestampEnd) }}</v-list-item-title>
                      </v-list-item-content>
                    </v-list-item>
                  </v-list-item-group>
                </v-list-item>
              </v-list>
            </v-card>
          </template>
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
      terrariumSession: [],
      liveModeEnabled: false,
      selectedSession: undefined,
      socket: undefined,
      socketTimer: undefined,

      from: "",
      to: "",
    };
  },
  watch: {
    selectedSession(val) {
      if (!this.isOpen) return;
      EventBus.$emit("filterUpdated", {});
      this.getSensorsMeasures();
    },
  },
  mounted() {
    let self = this;

    EventBus.$on("changeDialogState", (value) => {
      self.isOpen = value.visibility;
      self.terrariumName = value.terrariumName;
      self.terrariumId = value.terrariumId;
      self.terrariumSensors = value.sensorsData;
      self.terrariumSession = value.Sessions;

      const momentDate = moment.utc(new Date());
      self.to = momentDate.clone().endOf("day").format("YYYY-MM-DD HH:mm");
      self.from = moment(
        new Date().toISOString().substr(0, 10),
        "YYYY-MM-DD HH:mm"
      )
        .subtract(3, "months")
        .format("YYYY-MM-DD HH:mm");

      self.getSensorsMeasures();
    });

    EventBus.$on("filterUpdated", (value) => {
      self.clearChart();
      if (value.onlyLast != undefined && !value.onlyLast) {
        self.stopLiveChart();
      } else if (value.onlyLast != undefined && value.onlyLast) {
        self.startLiveChart();
      } else if (value.to != undefined && value.from != undefined) {
        self.to = value.to;
        self.from = value.from;
        self.getSensorsMeasures();
      }
    });
  },

  methods: {
    close() {
      this.terrariumName = "";
      this.terrariumId = "";
      this.terrariumSensors = [];
      this.isOpen = false;
      this.liveModeEnabled = false;
      this.selectedSession = undefined;
      if (this.socket) this.socket.close();
      if (this.cancelTokenSource != undefined) this.cancelTokenSource.cancel();
      if ( this.socketTimer != undefined) clearInterval(this.socketTimer);
    },
    formatDate(date) {
      if (date == "") return "--/--/--/ --:--";

      return moment(date).format("YYYY/MM/DD HH:mm");
    },
    startLiveChart() {
      let self = this;
      self.liveModeEnabled = true;
      this.socket = new WebSocket(
        `${process.env.VUE_APP_WS_URL}/data/terrariums/ws?id=${this.terrariumId}`
      );
      this.socket.onerror = (err) => {
        console.error(err);
        this.close();
      };
      this.socket.onmessage = (messageEvent) => {
        this.updateSensorsMeasuresFromLiveSocket(JSON.parse(messageEvent.data));
      };
      this.socketTimer = setInterval(() => {
        if (self.socket.readyState === WebSocket.CLOSED || self.socket.readyState === WebSocket.CLOSING) {
          console.log('WebSocket disconnected. Attempting to reconnect...');
          initWebSocket();
        } else {
          this.socket.send("pong");
        }
      }, 5000);
    },
    stopLiveChart() {
      let self = this;
      self.liveModeEnabled = false;
      clearInterval(this.socketTimer);
      this.socketTimer = undefined;
      EventBus.$emit("liveModeStop", { terrariumId: self.terrariumId });
      if (this.socket) {
        this.socket.close();
      }
    },
    clearChart() {
      EventBus.$emit("updateChart", {
        data: null,
      });
    },
    updateSensorsMeasuresFromLiveSocket(data) {
      let temp = [];
      this.terrariumSensors.forEach((el) => {
        temp[el.ID] = [];
      });
      data.forEach((el) => {
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
          liveMode: this.liveModeEnabled,
          sensors: this.terrariumSensors,
        });
      });
    },
    getSensorsMeasures() {
      let self = this;
      let requestState = self.liveModeEnabled ? true : false;
      let selectedSessionKey = "";

      if (
        self?.terrariumSession?.length > 0 &&
        self.selectedSession !== undefined
      ) {
        selectedSessionKey =
          self.terrariumSession[self.selectedSession].SessionKey;
      }

      Vue.axios
        .get(
          "/data/measures?TerrariumID=" +
          self.terrariumId +
          "&From=" +
          self.from +
          "&To=" +
          self.to +
          "&LastUpdateOnly=" +
          self.liveModeEnabled +
          "&SessionKey=" +
          selectedSessionKey
        )
        .then((res) => {
          let temp = [];

          if (self.liveModeEnabled != requestState) {
            return;
          }

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
          } else {
            self.clearChart();
          }
        })
        .catch((err) => {
          self.loading = false;
        });
    },
  },
};
</script>

