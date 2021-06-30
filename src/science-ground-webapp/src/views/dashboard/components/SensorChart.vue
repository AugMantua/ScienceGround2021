<template>
  <v-card elevation="0" outlined>
    <apexchart type="line" :options="options" :series="series"></apexchart>
  </v-card>
</template>

<script>
import VueApexCharts from "vue-apexcharts";
import Vue from "vue";
import { EventBus } from "../../../main";

export default {
  name: "SensorChart",
  components: {
    apexchart: VueApexCharts,
  },
  props: ["sensorDatas", "terrariumId", "dateTimeFilters"],
  data() {
    return {
      options: {
        chart: {
          id: "vuechart-example",
        },
        xaxis: {
          labels: {
            show: false,
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
      self.getTerrariunDatas(value.from, value.to);
    });
  },

  methods: {
    getTerrariunDatas(from, to) {
      let self = this;

      let data = {
        TerrariumId: self.terrariumId,
        From: from,
        To: to,
        SensorId: self.sensorDatas.SensorID,
      };

      Vue.axios
        .post("/data/measures/get", data)
        .then((res) => {
          console.log(res);

          let temp = [];

          res.data.forEach((element) => {
            temp.push({
              x: element.Timestamp,
              y: element.Value,
            });
          });

          self.series = [{
            data: temp
          }]
        })
        .catch((err) => {
          console.log(err);
        });
    },
  },
};
</script>