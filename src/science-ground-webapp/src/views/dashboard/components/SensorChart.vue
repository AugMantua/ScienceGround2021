<template>
  <v-card elevation="0" outlined :loading="loading">
    <apexchart
      type="line"
      :options="options"
      :series="series"
      ref="sensorChart"
    ></apexchart>
  </v-card>
</template>

<script>
import VueApexCharts from "vue-apexcharts";
import Vue from "vue";
import { EventBus } from "../../../main";
import moment from "moment";
export default {
  name: "SensorChart",
  components: {
    apexchart: VueApexCharts,
  },
  props: ["sensorDatas", "terrariumId", "dateTimeFilters"],
  data() {
    return {
      loading: true,
      toFilter: "",
      fromFilter: "",

      liveMode: false,
      liveTimer: null,

      options: {
        chart: {
          id: "vuechart-example",
        },
        animations: {
          enabled: true,
          easing: "linear",
          dynamicAnimation: {
            speed: 1000,
          },
        },
        xaxis: {
          labels: {
            show: false,
          },
        },
        yaxis: {
          title: {
            text: "",
          },
        },
      },
      series: [
        {
          name: "series-1",
          data: [],
        },
      ],
    };
  },

  mounted() {
    let self = this;

    EventBus.$on("updateChart", (value) => {
      if (value.onlyLast == undefined) {
        self.toFilter = value.to;
        self.fromFilter = value.from;

        if (self.liveMode) {
          self.liveMode = false;
          clearInterval(self.liveTimer);
          self.loading = false;
        }
      } else if (value.onlyLast) {
        self.startLiveChart();
      }

      self.getTerrariunDatas(value.from, value.to);
    });

    self.toFilter = new Date().toISOString().substr(0, 10);
    self.fromFilter = moment(
      new Date().toISOString().substr(0, 10),
      "YYYY-MM-DD"
    )
      .subtract(3, "months")
      .format("YYYY-MM-DD");
    self.getTerrariunDatas();
  },
  beforeDestroy() {
    EventBus.$off("updateChart");
    if (this.liveMode) {
      clearInterval(this.liveTimer);
      this.loading = false;
    }
  },
  methods: {
    startLiveChart() {
      let self = this;
      self.liveMode = true;
      self.loading = false;
      this.liveTimer = setInterval(() => {
        self.getTerrariunDatas();
      }, 1000);
    },
    reloadAllData(res) {
      let self = this;
      let temp = [];

      if (res.data.data == null) {
        self.series = [
          {
            data: [],
          },
        ];
        self.loading = false;
        return;
      }

      res.data.data.forEach((element) => {
        temp.push({
          x: element.Timestamp,
          y: element.Value,
        });
      });

      self.series = [
        {
          data: temp,
          name:
            self.sensorDatas.TypeOfMeasure +
            " (" +
            self.sensorDatas.Extra_data +
            ")",
        },
      ];
      self.$refs.sensorChart.updateOptions({
        yaxis: {
          title: {
            text:
              self.sensorDatas.TypeOfMeasure +
              " (" +
              self.sensorDatas.Extra_data +
              ")",
          },
        },
      });
      self.loading = false;
    },
    getTerrariunDatas() {
      let self = this;
      if (!self.liveMode) self.loading = true;

      Vue.axios
        .get(
          "/data/measures/get?TerrariumID=" +
            self.terrariumId +
            "&From=" +
            self.fromFilter +
            "&To=" +
            self.toFilter +
            "&SensorID=" +
            self.sensorDatas.ID +
            "&LastUpdateOnly=" +
            self.liveMode
        )
        .then((res) => {
          console.log(res);

          if (!self.liveMode) {
            self.reloadAllData(res);
          } else {
            if (
              self.series[0].data[self.series[0].data.length - 1].Timestamp !=
              res.data.data[0].Timestamp
            ) {
              self.series[0].data.push({
                x: res.data.data[0].Timestamp,
                y: res.data.data[0].Value,
              });

              self.$refs.sensorChart.updateSeries([
                {
                  data: self.series[0].data,
                },
              ]);
            }
          }
        })
        .catch((err) => {
          self.loading = false;
        });
    },
  },
};
</script>