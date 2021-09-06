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
import { EventBus } from "../../../main";

export default {
  name: "SensorChart",
  components: {
    apexchart: VueApexCharts,
  },
  props: ["sensorDatas"],
  data() {
    return {
      loading: true,
      toFilter: "",

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
      series: [],
    };
  },

  mounted() {
    let self = this;

    EventBus.$on("updateChart", (value) => {
      self.reloadAllData(value);
    });
    EventBus.$on("filterUpdated", (value) => {
      self.loading = true;
    });
  },
  beforeDestroy() {
    EventBus.$off("updateChart");
  },
  methods: {
    reloadAllData(res) {
      let self = this;

      if (res.data == null) {
        self.loading = false;
        self.series = [];
        return;
      }

      // different chart needed to be update
      if (
        self.sensorDatas != null &&
        res.key != null &&
        res.key != self.sensorDatas.ID
      ) {
        return;
      }

      // update multi series chart
      if ((self.series.length < res.sensors.length || !res.liveMode)  && self.sensorDatas == null) {
        self.loading = false;

        let seriesName = "";

        res.sensors.forEach((el) => {
          if (el.ID == res.key) {
            seriesName = el.TypeOfMeasure + " " + el.Extra_data;
          }
        });

        self.series.push({
          data: res.data,
          name: seriesName,
        });
        return;
      }

      // update series on mode live
      if (res.liveMode && self.series.length > 0) {
        self.loading = false;
        self.addDataToSerie(res.key, res.sensors, res.data);
        return;
      }

      self.series.push({
        data: res.data,
        name:
          self.sensorDatas.TypeOfMeasure +
          " (" +
          self.sensorDatas.Extra_data +
          ")",
      });

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
    addDataToSerie(sensorKey, sensorsList, updateData) {
      let seriesPos = 0;

      let self = this;
      if (self.sensorDatas == null) {
        sensorsList.forEach((el, index) => {
          if (el.ID == sensorKey) {
            seriesPos = index;
          }
        });
      }

      let seriesNumElem = 0;

      if (self.series[seriesPos] != undefined) {
        seriesNumElem = self.series[seriesPos].data.length;
      } 

      if (
        seriesNumElem == 0 ||
        self.series[seriesPos].data[seriesNumElem - 1].x != updateData[0].x
      ) {
        self.series[seriesPos].data.push(updateData[0]);

        self.$refs.sensorChart.updateSeries(self.series);
      }
    },
  },
};
</script>