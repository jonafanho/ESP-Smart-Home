const PORT_COUNT = 4;
const DEFAULT_RULES = [...Array(PORT_COUNT)].map((u, i) => []);
const CONDITIONS = {
	"time": {
		name: "The time of day is",
		icon: "⏰",
		min: 0,
		max: 1440,
		conversion: "time"
	},
	"dayOfWeek": {
		name: "The day of week is",
		icon: "📅",
		values: ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
	},
	"temperature": {
		name: "The temperature is",
		icon: "🌡️",
		min: -273,
		max: 1000,
		unit: "&deg;C"
	},
	"humidity": {
		name: "The humidity is",
		icon: "💧",
		min: 0,
		max: 100,
		unit: "%"
	},
	"light": {
		name: "The light level is",
		icon: "💡",
		min: 0,
		max: 100,
		unit: "%"
	},
	"proximity": {
		name: "Motion detector",
		icon: "🍃",
		values: ["Motion detected"]
	}
};

function setup() {
	"use strict";

	class MainScreen extends React.Component {

		constructor(props) {
			super(props);
			this.editOrSave = this.editOrSave.bind(this);
			this.state = {edit: false, rules: Object.assign({}, DEFAULT_RULES)};
		}

		editOrSave(event) {
			const isEdit = this.state.edit;
			if (isEdit) {
				fetch(`/settings?`, {
					method: "POST",
					body: "",
					headers: {
						"Content-type": "application/json; charset=UTF-8"
					}
				}).then(response => response.json()).then(data => {
				}).catch(error => {
				});
			}
			this.setState({edit: !isEdit});
		}

		render() {
			return (
				<div>
					<h1>Smart Home Device Configuration</h1>
					<table>
						<tbody>
						{[...Array(PORT_COUNT)].map((u, i) =>
							<Port key={i} rules={this.state.rules[i]} index={i} edit={this.state.edit}/>
						)}
						</tbody>
					</table>
					<br/>
					<input
						className="input_button"
						type="submit"
						value={this.state.edit ? "Save" : "Edit"}
						onClick={this.editOrSave}
					/>
				</div>
			);
		}
	}

	class Port extends React.Component {

		constructor(props) {
			super(props);
			this.addRule = this.addRule.bind(this);
			this.addCondition = this.addCondition.bind(this);
			this.deleteCondition = this.deleteCondition.bind(this);
		}

		addRule(event) {
			if (this.canAddRule()) {
				this.props.rules.push({});
				this.setState({});
			}
		}

		addCondition(ruleIndex, conditionId, event) {
			this.props.rules[ruleIndex][conditionId] = Object.assign({}, CONDITIONS[conditionId]);
			this.setState({});
		}

		deleteCondition(ruleIndex, conditionId, event) {
			delete this.props.rules[ruleIndex][conditionId];
			if (Object.keys(this.props.rules[ruleIndex]).length === 0) {
				this.props.rules.splice(ruleIndex, 1);
			}
			this.setState({});
		}

		canAddRule() {
			const ruleCount = this.props.rules.length;
			return ruleCount === 0 || Object.keys(this.props.rules[ruleCount - 1]).length > 0;
		}

		getAvailableConditions(ruleIndex) {
			return Object.keys(CONDITIONS).filter(key => !(key in this.props.rules[ruleIndex]));
		}

		portName(index) {
			const {rules, edit} = this.props;
			return (
				<td className="column_ports" rowSpan={Math.max(rules.length, 1)}>
					<h2>Port {index + 1}</h2>
					{edit && this.canAddRule() ? <a onClick={this.addRule}>Add Rule</a> : null}
				</td>
			);
		}

		render() {
			const {rules, index, edit} = this.props;
			const ruleCount = rules.length;
			if (ruleCount === 0) {
				return <tr>{this.portName(index)}</tr>;
			} else {
				return (
					<>
						{[...Array(ruleCount)].map((u, ruleIndex) =>
							<tr key={ruleIndex}>
								{ruleIndex === 0 ? this.portName(index) : null}
								<td className="column_conditions">
									{Object.keys(rules[ruleIndex]).map(conditionId =>
										<Condition
											key={`condition_${ruleIndex}_${conditionId}`}
											conditionId={conditionId}
											condition={rules[ruleIndex][conditionId]}
											onDelete={() => this.deleteCondition(ruleIndex, conditionId)}
											edit={edit}
										/>
									)}
								</td>
								<td className="column_add_conditions">
									{edit ? this.getAvailableConditions(ruleIndex).map(conditionId =>
										<AddButton
											key={`add_${ruleIndex}_${conditionId}`}
											icon={CONDITIONS[conditionId]["icon"]}
											onClick={(event) => this.addCondition(ruleIndex, conditionId, event)}
										/>
									) : null}
								</td>
							</tr>
						)}
					</>
				);
			}
		}
	}

	function AddButton(props) {
		return (
			<div className="clickable" onClick={props["onClick"]}>
				<div className="add_button text">+</div>
				<div className="add_button">{props["icon"]}</div>
			</div>
		);
	}

	class Condition extends React.Component {

		constructor(props) {
			super(props);
			this.state = {};
		}

		render() {
			const {conditionId, condition, onDelete, edit} = this.props;
			const details = CONDITIONS[conditionId];
			const isDiscrete = "values" in details;
			return (
				<div className={`condition ${conditionId}`}>
					{edit ? <div className="clickable delete_button" onClick={onDelete}>✖</div> : null}
					{condition["icon"]} {details["name"]}
					<br/>
					{isDiscrete ?
						[...Array(details["values"].length)].map((u, index) =>
							<div key={index}>
								<label>
									<input className="input_checkbox" type="checkbox"/>
									{details["values"][index]}
								</label>
								<br/>
							</div>
						) :
						<select>
							<option value="lessThan">less than</option>
							<option value="greaterThan">greater than</option>
							<option value="inRange">between</option>
							<option value="outsideRange">not between</option>
						</select>
					}
				</div>
			);
		}
	}

	if (window.location.pathname !== "/") {
		window.location.pathname = "/";
	}
	ReactDOM.render(<MainScreen/>, document.querySelector("#react-root"));
}

setup();